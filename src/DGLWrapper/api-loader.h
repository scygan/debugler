#include <DGLCommon/gl-types.h>

enum LibraryFlags {
    LIBRARY_WGL = 1,
    LIBRARY_EGL = 2,
    LIBRARY_GL  = 4,

    LIBRARY_GL_EXT = 0,
    LIBRARY_EGL_EXT = 0,
    LIBRARY_WGL_EXT = 0,
};

void LoadOpenGLLibrary (const char* libraryName, int libraryFlags);

void* LoadOpenGLExtPointer(Entrypoint entryp);

void* EnsurePointer(Entrypoint entryp);


