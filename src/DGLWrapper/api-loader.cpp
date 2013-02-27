#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

#include <string>
#include <vector>

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

#ifdef USE_MHOOK
#include "mhook/mhook-lib/mhook.h"
#endif

#include "gl-wrappers.h"

#include <DGLCommon/os.h>

//here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
//use DIRECT_CALL(name) to call one of these pointers
LoadedPointer g_DirectPointers[Entrypoints_NUM] = {
#define FUNCTION_LIST_ELEMENT(name, type, library) { NULL, library},
    #include "../../dump/codegen/functionList.inl"
#undef FUNCTION_LIST_ELEMENT
};

HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(const char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

void* LoadOpenGLExtPointer(Entrypoint entryp) {
    if (!g_DirectPointers[entryp].ptr) {
        return g_DirectPointers[entryp].ptr = DIRECT_CALL(wglGetProcAddress)(GetEntryPointName(entryp));
    } else {
        return g_DirectPointers[entryp].ptr;
    }    
}

void LoadOpenGLLibrary(const char* libraryName, int libraryFlags) {

    std::vector<std::string> libSearchPath;

    char buffer[1000];
#ifndef _WIN64
    if (GetSystemWow64Directory(buffer, sizeof(buffer)) > 0) {
        //we are running 32bit app on 64 bit windows
        libSearchPath.push_back(buffer);
    }
#endif
    if (!openGLLibraryHandle) {
        if (GetSystemDirectory(buffer, sizeof(buffer)) > 0) {
            //we are running on native system (32 on 32 or 64 on 64)
            libSearchPath.push_back(buffer);
        }
    }

#ifndef _WIN64
    libSearchPath.push_back("C:\\Windows\\SysWOW64\\");
#endif
    libSearchPath.push_back("C:\\Windows\\System32\\");

    openGLLibraryHandle = NULL;
    for (size_t i = 0; i < libSearchPath.size() && !openGLLibraryHandle; i++) {
        openGLLibraryHandle = LoadLibrary((libSearchPath[i] + libraryName).c_str());
    }
    
    if (!openGLLibraryHandle) {
        MessageBox(0, "Cannot load library", "Cannot load OpenGL32.dll system library", MB_OK | MB_ICONSTOP);
        exit(EXIT_FAILURE);
    }

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

        if (g_DirectPointers[i].libraryMask & libraryFlags) {
            g_DirectPointers[i].ptr = LoadOpenGLPointer(GetEntryPointName(i));
        }

        if (g_DirectPointers[i].ptr) {
            //this entrypoint was loaded from OpenGL32.dll, detour it!
            void * hookPtr = getWrapperPointer(i);
#ifdef USE_DETOURS
            DetourAttach(&(PVOID&)g_DirectPointers[i].ptr, hookPtr);
#endif
#ifdef USE_MHOOK
            if (!Mhook_SetHook(&(PVOID&)g_DirectPointers[i].ptr, hookPtr)) {
                std::string error = "Cannot load OpenGL32.dll funcion ";
                error += GetEntryPointName(i);
                error += "().";
                MessageBox(0, error.c_str(), "Cannot hook library", MB_OK | MB_ICONSTOP);
                Os::terminate();
            }
#endif
        }
    }
#ifdef USE_DETOURS
    DetourTransactionCommit();
#endif

}


void* EnsurePointer(Entrypoint entryp) {
    if (g_DirectPointers[entryp].ptr || LoadOpenGLExtPointer(entryp)) {
        return g_DirectPointers[entryp].ptr;
    } else {
        std::string error = "Operation aborted, because the ";
        error += GetEntryPointName(entryp);
        error += " function is not available on current context. Try updating GPU drivers.";
        throw std::runtime_error(error);
    }
}


