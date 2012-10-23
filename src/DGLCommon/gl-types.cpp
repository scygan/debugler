#include "gl-types.h"

#define FUNCTION_LIST_ELEMENT(name, type) #name,
char* g_EntrypointNames[] = {
#include "../../dump/codegen/functionList.inl"
    ""
};
#undef FUNCTION_LIST_ELEMENT


char* GetEntryPointName(Entrypoint entrp) {
    return g_EntrypointNames[entrp];
}