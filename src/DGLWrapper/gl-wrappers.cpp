#include <cassert>
#include "gl-headers-inside.h"
#include "pointers.h"
#include "tracer.h"
#include "gl-wrappers.h"

extern "C" {
#include "../../dump/codegen/wrappers.inl"
};


#define FUNCTION_LIST_ELEMENT(name, type) &name,
const void * wrapperPtrs[] = {
    #include "../../dump/codegen/functionList.inl"
    NULL
};


const void* getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}