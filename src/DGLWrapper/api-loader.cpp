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


#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

#include <string>
#include <vector>
#include <stdexcept>

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

#ifdef USE_MHOOK
#include "mhook/mhook-lib/mhook.h"
#endif

#include "gl-wrappers.h"

#include <DGLCommon/os.h>

#ifndef _WIN32
#include "dl-intercept.h"
#endif

//here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
//use DIRECT_CALL(name) to call one of these pointers
LoadedPointer g_DirectPointers[Entrypoints_NUM] = {
#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) { NULL, library},
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) FUNC_LIST_ELEM_SUPPORTED(name, type, library)
    #include "codegen/functionList.inl"
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED
};

APILoader::APILoader():m_LoadedApiLibraries(LIBRARY_NONE),m_GlueLibrary(LIBRARY_NONE) {}

FUNC_PTR APILoader::loadGLPointer(LoadedLib library, Entrypoint entryp) {
#ifdef _WIN32
    return GetProcAddress((HINSTANCE)library, GetEntryPointName(entryp));
#else
    //(int) -> see http://www.trilithium.com/johan/2004/12/problem-with-dlsym/
    return reinterpret_cast<FUNC_PTR>((ptrdiff_t)dlsym(library, GetEntryPointName(entryp)));
#endif
}

bool APILoader::loadExtPointer(Entrypoint entryp) {
    if (!g_DirectPointers[entryp].ptr) {

        if (!m_GlueLibrary) {
            throw std::runtime_error("Trying to call *GetProcAdress, but no glue library loaded");
        }

        FUNC_PTR ptr = NULL;

        switch (m_GlueLibrary) {
#ifdef HAVE_LIBRARY_WGL
        case LIBRARY_WGL:
            ptr  = DIRECT_CALL(wglGetProcAddress)(GetEntryPointName(entryp));
            break;
#endif
#ifdef HAVE_LIBRARY_GLX
        case LIBRARY_GLX:
            ptr = reinterpret_cast<FUNC_PTR>(DIRECT_CALL(glXGetProcAddress)(
                        reinterpret_cast<const GLubyte*>(GetEntryPointName(entryp))));
            break;
#endif
        case LIBRARY_EGL:
            ptr = reinterpret_cast<FUNC_PTR>(DIRECT_CALL(eglGetProcAddress)(GetEntryPointName(entryp)));
            break;
        default:
            assert(!"unknown glue library");
        }
        g_DirectPointers[entryp].ptr = ptr;
    }
    return g_DirectPointers[entryp].ptr != NULL;
}

std::string APILoader::getLibraryName(ApiLibrary apiLibrary) {
    switch (apiLibrary) {
        case LIBRARY_EGL:
#ifdef _WIN32
            return "libEGL.dll";
#else
    #ifdef __ANDROID__
            //on Android libraries are not opened by SONAME
            return "libEGL.so";
    #else
            return "libEGL.so.1";
    #endif
#endif
        case LIBRARY_GL:
        case LIBRARY_WGL:
        case LIBRARY_GLX:
#ifdef _WIN32
            return "opengl32.dll";
#else
            return "libGL.so.1";
#endif
        case LIBRARY_ES2:
        case LIBRARY_ES3:
#ifdef _WIN32
            return "libGLESv2.dll";
#else
    #ifdef __ANDROID__
            //on Android libraries are not opened by SONAME
            return "libGLESv2.so";
    #else
            return "libGLESv2.so.1";
    #endif
#endif
        default:
            assert(!"unknown library");
            throw std::runtime_error("Unknown GL library name");
    }
}

bool APILoader::isLibGL(const char* name) {
    std::string nameStr(name);
    return nameStr.find("libGL") != std::string::npos ||
        nameStr.find("libGLES") != std::string::npos ||
        nameStr.find("libEGL") != std::string::npos;
}

void APILoader::setPointer(Entrypoint entryp, FUNC_PTR direct) {
    if (entryp < NO_ENTRYPOINT) {
         g_DirectPointers[entryp].ptr = direct;
    }
}

void APILoader::loadLibrary(ApiLibrary apiLibrary) {

    std::string libraryName = getLibraryName(apiLibrary);

    if (m_LoadedLibraries.find(libraryName) == m_LoadedLibraries.end()) {
        std::vector<std::string> libSearchPath;

        LoadedLib openGLLibraryHandle = NULL;

        libSearchPath.push_back("");
#ifdef _WIN32
        char buffer[1000];
#ifndef _WIN64
        if (GetSystemWow64Directory(buffer, sizeof(buffer)) > 0) {
            //we are running 32bit app on 64 bit windows
            libSearchPath.push_back(buffer + std::string("\\"));
        }
#endif
        if (!openGLLibraryHandle) {
            if (GetSystemDirectory(buffer, sizeof(buffer)) > 0) {
                //we are running on native system (32 on 32 or 64 on 64)
                libSearchPath.push_back(buffer + std::string("\\"));
            }
        }
#ifndef _WIN64
        libSearchPath.push_back("C:\\Windows\\SysWOW64\\");
#endif
        libSearchPath.push_back("C:\\Windows\\System32\\");
        libSearchPath.push_back(".");
#endif

        for (size_t i = 0; i < libSearchPath.size() && !openGLLibraryHandle; i++) {
#ifdef _WIN32
            openGLLibraryHandle = (LoadedLib)LoadLibrary((libSearchPath[i] + libraryName).c_str());
#else
            openGLLibraryHandle = dlopen((libSearchPath[i] + libraryName).c_str(), RTLD_NOW);
#endif
        }

        if (!openGLLibraryHandle) {
            std::string msg = std::string("Cannot load ") + libraryName + "  system library";
            Os::fatal(msg.c_str());
        } else {
            m_LoadedLibraries[libraryName] = openGLLibraryHandle;
        }
    }

    LoadedLib library = m_LoadedLibraries[libraryName];


    //we use MS Detours only on win32, on x64 mHook is used
#ifdef USE_DETOURS
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
#endif
    //g_DirectPointers is now filled with opengl32.dll pointers
    // we will now detour (hook) them all, so g_DirectPointers will still lead to original opengl32.dll, but
    //application will always call us. 
    for (int i = 0; i < Entrypoints_NUM; i++) {
        if (!(g_DirectPointers[i].libraryMask & apiLibrary)) {
            //Do not load - entrypoint does not belong to currently loaded API
            continue;
        }

        if (m_LoadedApiLibraries & g_DirectPointers[i].libraryMask) {
            //Do not load - entrypoint belongs to already loaded API
            continue;
        }

        //this sets g_DirectPointers[i].ptr
        setPointer(i, loadGLPointer(library, i));

        if (g_DirectPointers[i].ptr) {
            //this entrypoint was loaded from OpenGL32.dll, detour it!
#if defined(USE_DETOURS) || defined(USE_MHOOK)            
            FUNC_PTR * hookPtr = getWrapperPointer(i);
#endif            
#ifdef USE_DETOURS
            DetourAttach(&(PVOID&)g_DirectPointers[i].ptr, hookPtr);
#endif
#ifdef USE_MHOOK
            if (!Mhook_SetHook(&(PVOID&)g_DirectPointers[i].ptr, hookPtr)) {
                Os::fatal("Cannot hook %s() function.", GetEntryPointName(i));
            }
#endif
        }
    }
#ifdef USE_DETOURS
    DetourTransactionCommit();
#endif
    if (apiLibrary == LIBRARY_EGL || apiLibrary == LIBRARY_WGL || apiLibrary == LIBRARY_GLX)
        m_GlueLibrary = apiLibrary;

    m_LoadedApiLibraries |= apiLibrary;
}

FUNC_PTR APILoader::ensurePointer(Entrypoint entryp) {
    if (g_DirectPointers[entryp].ptr || loadExtPointer(entryp)) {
        return g_DirectPointers[entryp].ptr;
    } else {
        std::string error = "Operation aborted, because the ";
        error += GetEntryPointName(entryp);
        error += " function is not available on current context. Try updating GPU drivers.";
        throw std::runtime_error(error);
    }
}


APILoader g_ApiLoader;
