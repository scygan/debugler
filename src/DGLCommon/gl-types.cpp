#include "gl-types.h"

#include<map>
#include<string>

namespace {
#define FUNCTION_LIST_ELEMENT(name, type) #name,
    char* g_EntrypointNames[] = {
#include "../../dump/codegen/functionList.inl"
        ""
    };
#undef FUNCTION_LIST_ELEMENT

    std::map<std::string, Entrypoint> g_EntryPointNameToEnum;
    
    void ensureEnumMapIntialized() {
        if (!g_EntryPointNameToEnum.size()) {
            for (Entrypoint e = 0; e < Entrypoints_NUM; e++) {
                g_EntryPointNameToEnum[GetEntryPointName(e)] = e;
            }
        }
    }
}

char* GetEntryPointName(Entrypoint entryp) {
    return g_EntrypointNames[entryp];
}

Entrypoint GetEntryPointEnum(const char* name) {
    ensureEnumMapIntialized();
    std::map<std::string, Entrypoint>::iterator ret = g_EntryPointNameToEnum.find(name);
    if (ret == g_EntryPointNameToEnum.end())
        return NO_ENTRYPOINT;
    return ret->second;
}