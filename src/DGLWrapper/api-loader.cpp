#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"

//here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
//use DIRECT_CALL(name) to call one of these pointers
void* g_DirectPointers[Entrypoints_NUM];

HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

void LoadOpenGLExtPointer(Entrypoint entryp) {
    //this is where we store the direct ptr
    if (!g_DirectPointers[entryp])
        g_DirectPointers[entryp] = DIRECT_CALL(wglGetProcAddress)(GetEntryPointName(entryp));
}

#define FUNCTION_LIST_ELEMENT(name, type) POINTER(name) = LoadOpenGLPointer(#name);
void LoadOpenGLPointers () {
    #include "../../dump/codegen/functionList.inl"
}
#undef FUNCTION_LIST_ELEMENT

void LoadOpenGLLibrary() {
    openGLLibraryHandle = LoadLibrary("C:\\Windows\\SysWOW64\\opengl32.dll");
    assert(openGLLibraryHandle);
    if (!openGLLibraryHandle) {
        MessageBox(0, "Cannot load library", "Cannot Load library OpenGL32.dll", MB_OK | MB_ICONSTOP);
        exit(EXIT_FAILURE);
    }

    LoadOpenGLPointers();
}



