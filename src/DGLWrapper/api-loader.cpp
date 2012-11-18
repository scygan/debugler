#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

#include <string>

//here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
//use DIRECT_CALL(name) to call one of these pointers
void* g_DirectPointers[Entrypoints_NUM];

HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

void* LoadOpenGLExtPointer(Entrypoint entryp) {
    return g_DirectPointers[entryp] = DIRECT_CALL(wglGetProcAddress)(GetEntryPointName(entryp));
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


