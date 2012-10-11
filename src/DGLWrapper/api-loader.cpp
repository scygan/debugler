#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"


#define PTR_PREFIX
#include "../codegen/output/pointers.inl"


HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

#define PTR_LOAD(X) LoadOpenGLPointer(#X)
void LoadOpenGLPointers () {
    #include "../codegen/output/pointers_load.inl"
}

void LoadOpenGLLibrary() {
    openGLLibraryHandle = LoadLibrary("C:\\Windows\\SysWOW64\\opengl32.dll");
    assert(openGLLibraryHandle);
    if (!openGLLibraryHandle) {
        MessageBox(0, "Cannot load library", "Cannot Load library OpenGL32.dll", MB_OK | MB_ICONSTOP);
        exit(EXIT_FAILURE);
    }

    LoadOpenGLPointers();
}



