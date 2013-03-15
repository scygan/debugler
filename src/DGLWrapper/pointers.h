#include <DGLCommon/gl-headers.h>
#include <DGLCommon/gl-types.h>

#include "codegen/nonExtTypedefs.inl"

//POINTER_TYPE(X) returns type of function pointer for entrypoint X. The actual definitions are generated from codegen output
//For entrypoints unsupported on given platform bare void* is returned. 
#define POINTER_TYPE(X) X##_Type

#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) typedef type POINTER_TYPE(name);
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) typedef void* POINTER_TYPE(name);
#include "codegen/functionList.inl"
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

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


