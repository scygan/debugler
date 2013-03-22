// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GLWRAPPER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GLWRAPPER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef _WIN32
#define DGLWRAPPER_API __declspec(dllexport)
#else
#define DGLWRAPPER_API 
#endif


//some auto-configuration for DGLwrapper: 


//library coverage (loaded and wrapped):
#define HAVE_LIBRARY_GL
#define HAVE_LIBRARY_GL_EXT
#define HAVE_LIBRARY_EGL
#define HAVE_LIBRARY_EGL_EXT
#ifdef _WIN32
    #define HAVE_LIBRARY_WGL
    #define HAVE_LIBRARY_WGL_EXT
#endif


//binary interception method
#ifdef _WIN32
    #ifdef _WIN64
        #define USE_MHOOK
    #else
        #define USE_DETOURS
    #endif
#endif
