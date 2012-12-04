#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

#include <string>
#include "detours/detours.h"
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
    openGLLibraryHandle = LoadLibrary("C:\\Windows\\SysWOW64\\opengl32.dll");
    if (!openGLLibraryHandle) {
        openGLLibraryHandle = LoadLibrary("C:\\Windows\\System32\\opengl32.dll");
    }

    assert(openGLLibraryHandle);
    if (!openGLLibraryHandle) {
        MessageBox(0, "Cannot load library", "Cannot Load library OpenGL32.dll", MB_OK | MB_ICONSTOP);
        exit(EXIT_FAILURE);
    }

    LoadOpenGLPointers();

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


