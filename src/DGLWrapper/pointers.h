#include <DGLCommon/gl-headers.h>
#include <DGLCommon/gl-types.h>

#include "codegen/nonExtTypedefs.inl"

//these crazy macros are sometimes used to split FUNCTION_LIST_ELEMENT(..., library) into FUNCTION_LIST_ELEMENT_SUPPORTED and FUNCTION_LIST_ELEMENT_UNSUPPORTED
#ifdef HAVE_LIBRARY_GL
#define FUNCTION_LIST_ELEMENT_LIBRARY_GL(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_GL(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif
#ifdef HAVE_LIBRARY_GL_EXT
#define FUNCTION_LIST_ELEMENT_LIBRARY_GL_EXT(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_GL_EXT(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif
#ifdef HAVE_LIBRARY_EGL
#define FUNCTION_LIST_ELEMENT_LIBRARY_EGL(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_EGL(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif
#ifdef HAVE_LIBRARY_EGL_EXT
#define FUNCTION_LIST_ELEMENT_LIBRARY_EGL_EXT(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_EGL_EXT(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif
#ifdef HAVE_LIBRARY_WGL
#define FUNCTION_LIST_ELEMENT_LIBRARY_WGL(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_WGL(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif
#ifdef HAVE_LIBRARY_WGL_EXT
#define FUNCTION_LIST_ELEMENT_LIBRARY_WGL_EXT(name, type) FUNCTION_LIST_ELEMENT_SUPPORTED(name, type)
#else
#define FUNCTION_LIST_ELEMENT_LIBRARY_WGL_EXT(name, type) FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type)
#endif


//POINTER_TYPE(X) returns type of function pointer for entrypoint X. The actual definitions are generated from codegen output
//For entrypoints unsupported on given platform bare void* is returned. 
#define POINTER_TYPE(X) X##_Type

#define FUNCTION_LIST_ELEMENT(name, type, library) FUNCTION_LIST_ELEMENT_##library(name, type)
#define FUNCTION_LIST_ELEMENT_SUPPORTED(name, type) typedef type POINTER_TYPE(name);
#define FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type) typedef void* POINTER_TYPE(name);
#include "codegen/functionList.inl"
#undef FUNCTION_LIST_ELEMENT
#undef FUNCTION_LIST_ELEMENT_SUPPORTED
#undef FUNCTION_LIST_ELEMENT_UNSUPPORTED

//DIRECT_CALL(X) can be used to directly call entrypoint X, like DIRECT_CALL(glEnable)(GL_BLEND).
#define DIRECT_CALL(X) (*(POINTER_TYPE(X))POINTER(X))
#define POINTER(X) g_DirectPointers[X##_Call].ptr

//DIRECT_CALL_CHECKED(X) can be used call entrypoint X with NULL-checking, like DIRECT_CALL_CHK(glEnable)(GL_BLEND).
//will throw exception on NULL
#define DIRECT_CALL_CHK(X) (*(POINTER_TYPE(X))POINTER_CHECKED(X))
#define POINTER_CHECKED(X) EnsurePointer(X##_Call)


struct LoadedPointer {
    void* ptr;
    int libraryMask;
};

extern LoadedPointer g_DirectPointers[Entrypoints_NUM];


