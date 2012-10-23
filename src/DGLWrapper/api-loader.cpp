#include <cstdlib>
#include <cassert>

#include "api-loader.h"
#include "pointers.h"


#define FUNCTION_LIST_ELEMENT(name, type) type POINTER(name);
#include "../../dump/codegen/functionList.inl"
#undef FUNCTION_LIST_ELEMENT


HINSTANCE  openGLLibraryHandle; 

void * LoadOpenGLPointer(char* name) {
    return GetProcAddress(openGLLibraryHandle, name);
}

#define FUNCTION_LIST_ELEMENT(name, type) POINTER(name) = (type) LoadOpenGLPointer(#name);
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



