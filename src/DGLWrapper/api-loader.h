#include <DGLCommon/gl-types.h>

enum LibraryFlags {
    LIBRARY_WGL = 1,
    LIBRARY_EGL = 2,
    LIBRARY_GL  = 4,
};

void LoadOpenGLLibrary (const char* libraryName, int libraryFlags);

void* LoadOpenGLExtPointer(Entrypoint entryp);

void* EnsurePointer(Entrypoint entryp);

#ifdef _WIN32
#ifdef _WIN64
#define USE_MHOOK
#else
#define USE_DETOURS
#endif
#endif