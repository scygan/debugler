#include <cassert>
#include "gl-headers-inside.h"
#include "pointers.h"
#include "tracer.h"
#include "gl-wrappers.h"

extern "C" {
#include "codegen/wrappers.inl"
};


#define FUNCTION_LIST_ELEMENT(name, type, library) FUNCTION_LIST_ELEMENT_##library(name, type)
#define FUNCTION_LIST_ELEMENT_SUPPORTED(name, type) (void*)&name,
#define FUNCTION_LIST_ELEMENT_UNSUPPORTED(name, type) NULL,
void * wrapperPtrs[] = {
    #include "codegen/functionList.inl"
    NULL
};
#undef FUNCTION_LIST_ELEMENT
#undef FUNCTION_LIST_ELEMENT_SUPPORTED
#undef FUNCTION_LIST_ELEMENT_UNSUPPORTED


void* getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}
