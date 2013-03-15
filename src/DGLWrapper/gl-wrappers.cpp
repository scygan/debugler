#include <cassert>
#include "gl-headers-inside.h"
#include "pointers.h"
#include "tracer.h"
#include "gl-wrappers.h"

extern "C" {
#include "codegen/wrappers.inl"
};


#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) (void*)&name,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) NULL,
void * wrapperPtrs[] = {
    #include "codegen/functionList.inl"
    NULL
};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED


void* getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}
