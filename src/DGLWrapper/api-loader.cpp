#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

#include <string>
#include <vector>
#ifndef _WIN64
#include "detours/detours.h"
#else
#include "mhook/mhook-lib/mhook.h"
#endif
#include "gl-wrappers.h"

//here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
//use DIRECT_CALL(name) to call one of these pointers
void* g_DirectPointers[Entrypoints_NUM];

HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

void* LoadOpenGLExtPointer(Entrypoint entryp) {
    if (!g_DirectPointers[entryp]) {
        return g_DirectPointers[entryp] = DIRECT_CALL(wglGetProcAddress)(GetEntryPointName(entryp));
    } else {
        return g_DirectPointers[entryp];
    }    
}

#define FUNCTION_LIST_ELEMENT(name, type) POINTER(name) = LoadOpenGLPointer(#name);
void LoadOpenGLPointers () {
    #include "../../dump/codegen/functionList.inl"
}
#undef FUNCTION_LIST_ELEMENT

void LoadOpenGLLibrary() {

    std::vector<std::string> libSearchPath;
    std::string libName = "\\opengl32.dll";

    char buffer[1000];
#ifndef _WIN64
    if (GetSystemWow64Directory(buffer, sizeof(buffer)) > 0) {
        //we are running 32bit app on 64 bit windows
        libSearchPath.push_back(buffer + libName);
    }
#endif
    if (!openGLLibraryHandle) {
        if (GetSystemDirectory(buffer, sizeof(buffer)) > 0) {
            //we are running on native system (32 on 32 or 64 on 64)
            libSearchPath.push_back(buffer + libName);
        }
    }

#ifndef _WIN64
    libSearchPath.push_back("C:\\Windows\\SysWOW64\\opengl32.dll");
#endif
    libSearchPath.push_back("C:\\Windows\\System32\\opengl32.dll");

    openGLLibraryHandle = NULL;
    for (size_t i = 0; i < libSearchPath.size() && !openGLLibraryHandle; i++) {
        openGLLibraryHandle = LoadLibrary(libSearchPath[i].c_str());
    }
    
    if (!openGLLibraryHandle) {
        MessageBox(0, "Cannot load library", "Cannot load OpenGL32.dll system library", MB_OK | MB_ICONSTOP);
        exit(EXIT_FAILURE);
    }

    LoadOpenGLPointers();

    //we use MS Detours only on win32, on x64 mHook is used
#ifndef _WIN64
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    //g_DirectPointers is now filled with opengl32.dll pointers
    // we will now detour (hook) them all, so g_DirectPointers will still lead to original opengl32.dll, but
    //application will always call us. 
    for (int i = 0; i < Entrypoints_NUM; i++) {
        if (g_DirectPointers[i]) {
            //this entrypoint was loaded from OpenGL32.dll, detour it!
            void * hookPtr = getWrapperPointer(i);
            DetourAttach(&(PVOID&)g_DirectPointers[i], hookPtr);
        }
    }
    DetourTransactionCommit();
#else 
    for (int i = 0; i < Entrypoints_NUM; i++) {
        if (g_DirectPointers[i]) {
            //this entrypoint was loaded from OpenGL32.dll, detour it!
            void * hookPtr = getWrapperPointer(i);
            Mhook_SetHook(&(PVOID&)g_DirectPointers[i], hookPtr);
        }
    }
#endif
}


void* EnsurePointer(Entrypoint entryp) {
    if (g_DirectPointers[entryp] || LoadOpenGLExtPointer(entryp)) {
        return g_DirectPointers[entryp];
    } else {
        std::string error = "Operation aborted, because the ";
        error += GetEntryPointName(entryp);
        error += " function is not available on current context. Try updating GPU drivers.";
        throw std::runtime_error(error);
    }
}


