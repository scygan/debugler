#include <DGLcommon/gl-headers.h>
#include <DGLcommon/gl-types.h>

#include "../../dump/codegen/nonExtTypedefs.inl"

#define POINTER_TYPE(X) X##_Type
#define FUNCTION_LIST_ELEMENT(name, type) typedef type POINTER_TYPE(name);
#include "../../dump/codegen/functionList.inl"
#undef FUNCTION_LIST_ELEMENT


#define POINTER(X) g_DirectPointers[X##_Call]
#define POINTER_CHECKED(X) EnsurePointer(X##_Call)
#define DIRECT_CALL(X) (*(POINTER_TYPE(X))POINTER(X))
#define DIRECT_CALL_CHK(X) (*(POINTER_TYPE(X))POINTER_CHECKED(X))

extern void* g_DirectPointers[Entrypoints_NUM];