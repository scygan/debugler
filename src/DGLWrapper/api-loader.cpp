/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "api-loader.h"

#include "pointers.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>

#include "gl-wrappers.h"

#include <DGLCommon/os.h>

#include "dl-intercept.h"
#include "hook.h"

#ifdef _WIN32
#define LIBGL_NAME "opengl32.dll"
#define LIBGLES1_NAME "libGLESv1_CM.dll"
#define LIBGLES2_NAME "libGLESv2.dll"
#define LIBEGL_NAME "libEGL.dll"
#define STRIP_VERSION(X) X
#elif defined(__ANDROID__)
// on Android libraries are not opened by SONAME
#define LIBGL_NAME "libGL.so"
#define LIBGLES1_NAME "libGLESv1_CM.so"
#define LIBGLES2_NAME "libGLESv2.so"
#define LIBEGL_NAME "libEGL.so"
#define STRIP_VERSION(X) X
#else
#define LIBGL_NAME "libGL.so.1"
#define LIBGLES1_NAME "libGLESv1_CM.so.1"
#define LIBGLES2_NAME "libGLESv2.so.2"
#define LIBEGL_NAME "libEGL.so.1"
#define STRIP_VERSION(X) \
    std::string(X).substr(0, std::string(X).length() - strlen(".1"))
#endif

// here direct pointers are kept (pointers to entrypoints exposed by underlying
// OpenGL32 implementation
// use DIRECT_CALL(name) to call one of these pointers
LoadedPointer g_DirectPointers[Entrypoints_NUM] = {
#define FUNC_LIST_ELEM_SUPPORTED(name, type, library, retVal, params) \
    { NULL, library }                                 \
    ,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library, retVal, params) \
    FUNC_LIST_ELEM_SUPPORTED(name, type, library, retVal, params)
#include "functionList.inl"
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED
};

APILoader::APILoader()
        : m_LoadedApiLibraries(LIBRARY_NONE), m_GlueLibrary(LIBRARY_NONE) {}

dgl_func_ptr APILoader::loadGLPointer(const DynamicLibrary& library, Entrypoint entryp) {
    return library.getFunction(GetEntryPointName(entryp));
}

bool APILoader::loadExtPointer(Entrypoint entryp) {
    if (!g_DirectPointers[entryp].ptr) {

        if (!m_GlueLibrary) {
            throw std::runtime_error(
                    "Trying to call *GetProcAdress, but no glue library "
                    "loaded");
        }

        dgl_func_ptr ptr = NULL;

        switch (m_GlueLibrary) {
#ifdef HAVE_LIBRARY_WGL
            case LIBRARY_WGL:
                ptr = reinterpret_cast<dgl_func_ptr>(DIRECT_CALL(wglGetProcAddress)(
                        GetEntryPointName(entryp)));
                break;
#endif
#ifdef HAVE_LIBRARY_GLX
            case LIBRARY_GLX:
                ptr = reinterpret_cast<dgl_func_ptr>(DIRECT_CALL(glXGetProcAddress)(
                        reinterpret_cast<const GLubyte*>(
                                GetEntryPointName(entryp))));
                break;
#endif
            case LIBRARY_EGL:
                ptr = reinterpret_cast<dgl_func_ptr>(DIRECT_CALL(eglGetProcAddress)(
                        GetEntryPointName(entryp)));
                break;
            default:
                DGL_ASSERT(!"unknown glue library");
        }
        g_DirectPointers[entryp].ptr = ptr;
    }
    return g_DirectPointers[entryp].ptr != NULL;
}

std::string APILoader::getLibraryName(ApiLibrary apiLibrary) {
    switch (apiLibrary) {
        case LIBRARY_EGL:
            return LIBEGL_NAME;
        case LIBRARY_GL:
        case LIBRARY_WGL:
        case LIBRARY_GLX:
            return LIBGL_NAME;
        case LIBRARY_ES1:
        case LIBRARY_ES1_ANDROID:
            return LIBGLES1_NAME;
        case LIBRARY_ES2:
        case LIBRARY_ES2_ANDROID:
        case LIBRARY_ES3:
            return LIBGLES2_NAME;
        case LIBRARY_WINGDI:            
            return "gdi32.dll";
        default:
            DGL_ASSERT(!"unknown library");
            throw std::runtime_error("Unknown GL library name");
    }
}

int APILoader::whichLibrary(const char* name) {
    std::string nameStr(name);

    if (nameStr.find(STRIP_VERSION(LIBGL_NAME)) != std::string::npos) {
        return LIBRARY_GL | LIBRARY_WGL | LIBRARY_GLX
            //as an exception assume app can load GL exts via dynamic library load
            //This is some because some Linux libraries contain much more syms than ABI specifies
                | LIBRARY_GL_EXT;
    }

    if (nameStr.find(STRIP_VERSION(LIBGLES1_NAME)) != std::string::npos) {
        return LIBRARY_ES1;
    }

    if (nameStr.find(STRIP_VERSION(LIBGLES2_NAME)) != std::string::npos) {
        return LIBRARY_ES2 | LIBRARY_ES3;
    }

    if (nameStr.find(STRIP_VERSION(LIBEGL_NAME)) != std::string::npos) {
        return LIBRARY_EGL;
    }

    return LIBRARY_NONE;
}

int APILoader::getEntryPointLibrary(Entrypoint entryp) {
    return g_DirectPointers[entryp].libraryMask;
}

void APILoader::setPointer(Entrypoint entryp, dgl_func_ptr direct) {
    if (entryp < NO_ENTRYPOINT && direct != nullptr) {
        g_DirectPointers[entryp].ptr = direct;
    }
}

void APILoader::loadLibraries(int apiLibraries) {
    int j = 1;
    for (size_t i = 1; i < sizeof(apiLibraries) * 8; i++) {
        if (apiLibraries & j)
            loadLibrary(ApiLibrary(j));
        j *= 2;
    }
}

void APILoader::loadDefaultLibraries(bool useEGL, int librariesMask, LoadMode mode) {
    if (useEGL) {
        loadLibrary(ApiLibrary(LIBRARY_EGL & librariesMask), mode);
    } else {
#ifdef _WIN32
        loadLibrary(ApiLibrary(LIBRARY_WGL & librariesMask), mode);
        loadLibrary(ApiLibrary(LIBRARY_WINGDI & librariesMask), mode);
#else
        loadLibrary(ApiLibrary(LIBRARY_GLX & librariesMask), mode);
#endif
        loadLibrary(ApiLibrary(LIBRARY_GL & librariesMask), mode);
    }
}

void APILoader::loadLibrary(ApiLibrary apiLibrary, LoadMode mode) {

    if (apiLibrary == LIBRARY_NONE) {
        return;
    }

    std::string libraryName = getLibraryName(apiLibrary);

    if (m_LoadedLibraries.find(libraryName) == m_LoadedLibraries.end()) {

#ifdef _WIN32
        if (mode == LoadMode::LAZY && !GetModuleHandle(libraryName.c_str())) {
            //Will load this later. This is used to avoid calling LoadLibrary() too early in Win.
            return;
        }
#endif

        std::vector<std::string> libSearchPath;

        DynamicLibrary* openGLLibraryHandle = nullptr;

        libSearchPath.push_back("");
#ifdef _WIN32
        char buffer[1000];
#ifndef _WIN64
        if (GetSystemWow64Directory(buffer, sizeof(buffer)) > 0) {
            // we are running 32bit app on 64 bit windows
            libSearchPath.push_back(buffer + std::string("\\"));
        }
#endif
        if (!openGLLibraryHandle) {
            if (GetSystemDirectory(buffer, sizeof(buffer)) > 0) {
                // we are running on native system (32 on 32 or 64 on 64)
                libSearchPath.push_back(buffer + std::string("\\"));
            }
        }
#ifndef _WIN64
        libSearchPath.push_back("C:\\Windows\\SysWOW64\\");
#endif
        libSearchPath.push_back("C:\\Windows\\System32\\");
        libSearchPath.push_back(".");
#endif

        for (size_t i = 0; i < libSearchPath.size() && !openGLLibraryHandle;
             i++) {
            openGLLibraryHandle = EarlyGlobalState::getDynLoader().getLibrary(
                    (libSearchPath[i] + libraryName).c_str());
        }

        if (!openGLLibraryHandle) {
            std::string msg = std::string("Cannot load ") + libraryName +
                              "  system library";
            Os::fatal(msg.c_str());
        } else {
            m_LoadedLibraries[libraryName] = openGLLibraryHandle;
        }
    }

    DynamicLibrary* library = m_LoadedLibraries[libraryName];

#ifdef _WIN32
    HookSession hookSession;
#endif

    // g_DirectPointers is now filled with opengl32.dll pointers
    // we will now detour (hook) them all, so g_DirectPointers will still lead
    // to original opengl32.dll, but
    // application will always call us.
    for (int i = 0; i < Entrypoints_NUM; i++) {
        if (!(g_DirectPointers[i].libraryMask & apiLibrary)) {
            // Do not load - entrypoint does not belong to currently loaded API
            continue;
        }

        if (m_LoadedApiLibraries & g_DirectPointers[i].libraryMask) {
            // Do not load - entrypoint belongs to already loaded API
            continue;
        }

        // this sets g_DirectPointers[i].ptr
        setPointer(i, loadGLPointer(*library, i));

        if (g_DirectPointers[i].ptr) {
// this entrypoint was loaded from OpenGL32.dll, detour it!
#ifdef _WIN32
            dgl_func_ptr hookPtr = getWrapperPointer(i);
            if (!hookSession.hook(&g_DirectPointers[i].ptr, hookPtr)) {
                Os::fatal("Cannot hook %s() function.", GetEntryPointName(i));
            }
#endif
        }
    }
    if (apiLibrary == LIBRARY_EGL || apiLibrary == LIBRARY_WGL ||
        apiLibrary == LIBRARY_GLX)
        m_GlueLibrary = apiLibrary;

    m_LoadedApiLibraries |= apiLibrary;
}

dgl_func_ptr APILoader::ensurePointer(Entrypoint entryp) {
    if (g_DirectPointers[entryp].ptr || loadExtPointer(entryp)) {
        return g_DirectPointers[entryp].ptr;
    } else {
        std::string error = "Operation aborted, because the ";
        error += GetEntryPointName(entryp);
        error +=
                " function is not available on current context. Try updating "
                "GPU "
                "drivers.";
        throw std::runtime_error(error);
    }
}
