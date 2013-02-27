#include <cassert>
#include "gl-headers-inside.h"
#include "pointers.h"
#include "tracer.h"
#include "gl-wrappers.h"

extern "C" {
#include "../../dump/codegen/wrappers.inl"
};


#define FUNCTION_LIST_ELEMENT(name, type, library) &name,
void * wrapperPtrs[] = {
    #include "../../dump/codegen/functionList.inl"
    NULL
};


void* getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}