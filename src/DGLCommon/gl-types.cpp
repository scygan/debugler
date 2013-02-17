#include "gl-types.h"

#include<map>
#include<set>
#include<sstream>

namespace {
#define FUNCTION_LIST_ELEMENT(name, type) #name,
    const char* g_EntrypointNames[] = {
#include "codegen/functionList.inl"
        ""
    };
#undef FUNCTION_LIST_ELEMENT

#define ENUM_LIST_ELEMENT(value) {#value, value},
    struct GLEnum {
        const char* name; 
        uint64_t value;
    }  g_GLEnums[] = {
#include "codegen/enum.inl"
        { NULL, 0 }
    };
#undef ENUM_LIST_ELEMENT

    std::map<std::string, Entrypoint> g_EntryPointNameToEnum;
    std::map<uint64_t, std::string> g_GLEnumValueToName;
    
    void ensureEnumMapIntialized() {
        if (!g_EntryPointNameToEnum.size()) {
            for (Entrypoint e = 0; e < Entrypoints_NUM; e++) {
                g_EntryPointNameToEnum[GetEntryPointName(e)] = e;
            }
        }
        if (!g_GLEnumValueToName.size()) {
            int i = 0; 
            while (g_GLEnums[i].name) {
                if (g_GLEnumValueToName.find(g_GLEnums[i].value) == g_GLEnumValueToName.end())
                    //do not overwrite any enum that already exist in the map:
                    //the first come GLenums are "older" - were defined earlier in OGL history. 
                    //the duplicates are usually some vendor extensions from the bottom glext.h
                    g_GLEnumValueToName[g_GLEnums[i].value] = g_GLEnums[i].name;
                i++;
            }
        }
    }
}

const char* GetEntryPointName(Entrypoint entryp) {
    return g_EntrypointNames[entryp];
}

Entrypoint GetEntryPointEnum(const char* name) {
    ensureEnumMapIntialized();
    std::map<std::string, Entrypoint>::iterator ret = g_EntryPointNameToEnum.find(name);
    if (ret == g_EntryPointNameToEnum.end())
        return NO_ENTRYPOINT;
    return ret->second;
}

std::string GetGLEnumName(uint64_t glEnum) {
    ensureEnumMapIntialized();
    std::map<uint64_t, std::string>::iterator ret = g_GLEnumValueToName.find(glEnum);
    if (ret == g_GLEnumValueToName.end()) {
        std::ostringstream tmp; 
        tmp << "0x" << std::hex << glEnum;
        return tmp.str();
    }
    return ret->second;
}

std::string GetShaderStageName(uint64_t glEnum) {
    switch (glEnum) {
        case GL_VERTEX_SHADER:
            return "Vertex";
        case GL_TESS_CONTROL_SHADER:
            return "Tesselation Control";
        case GL_TESS_EVALUATION_SHADER:
            return "Tesselation Evaluation";
        case GL_GEOMETRY_SHADER:
            return "Geometry";
        case GL_FRAGMENT_SHADER:
            return "Fragment";
        default:
            return GetGLEnumName(glEnum).c_str();
    }
}

std::string GetTextureTargetName(uint64_t glEnum) {
    switch (glEnum) {
    case GL_TEXTURE_1D:
        return "1D";
    case GL_TEXTURE_2D:
        return "2D";
    case GL_TEXTURE_3D:
        return "3D";
    case GL_TEXTURE_1D_ARRAY:
        return "1D Array";
    case GL_TEXTURE_2D_ARRAY:
        return "2D Array";
    case GL_TEXTURE_RECTANGLE:
        return "Rectangle";
    case GL_TEXTURE_CUBE_MAP:
        return "Cube Map";
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        return "Cube Map Array";
    case GL_TEXTURE_BUFFER:
        return "Buffer";
    case GL_TEXTURE_2D_MULTISAMPLE:
        return "2D MS";
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        return "2D MS Array";
    default:
        return GetGLEnumName(glEnum).c_str();
    }
}

namespace call_sets {
    Entrypoint frameDelims[] = {
         wglSwapBuffers_Call,
         wglSwapLayerBuffers_Call,
         wglSwapMultipleBuffers_Call
    };
    Entrypoint drawCalls[] = {
        glDrawElements_Call,
        glDrawElementsBaseVertex_Call,
        glDrawElementsIndirect_Call,
        glDrawElementsInstanced_Call,
        glDrawElementsInstancedARB_Call,
        glDrawElementsInstancedBaseInstance_Call,
        glDrawElementsInstancedBaseVertex_Call,
        glDrawElementsInstancedBaseVertexBaseInstance_Call,
        glDrawElementsInstancedEXT_Call,
        glMultiDrawElements_Call,
        glMultiDrawElementsBaseVertex_Call,
        glMultiDrawElementsEXT_Call,
        glMultiDrawElementsIndirect_Call,
        glMultiDrawElementsIndirectAMD_Call,
        glMultiModeDrawElementsIBM_Call,
        glDrawArrays_Call,
        glDrawArraysEXT_Call,
        glDrawArraysIndirect_Call,
        glDrawArraysInstanced_Call,
        glDrawArraysInstancedARB_Call,
        glDrawArraysInstancedBaseInstance_Call,
        glDrawArraysInstancedEXT_Call,
        glMultiDrawArrays_Call,
        glMultiDrawArraysEXT_Call,
        glMultiDrawArraysIndirect_Call,
        glMultiDrawArraysIndirectAMD_Call,
        glMultiModeDrawArraysIBM_Call,
        glClear_Call,
        glClearBufferfi_Call,
        glClearBufferfv_Call,
        glClearBufferiv_Call,
        glClearBufferuiv_Call,
        glDrawRangeElementArrayAPPLE_Call,
        glDrawRangeElementArrayATI_Call,
        glDrawRangeElements_Call,
        glDrawRangeElementsBaseVertex_Call,
        glDrawRangeElementsEXT_Call,
        glMultiDrawRangeElementArrayAPPLE_Call
    };

    std::set<Entrypoint> drawCallSet(&drawCalls[0], &drawCalls[sizeof(drawCalls)/sizeof(drawCalls[0])]);
    std::set<Entrypoint> frameDelimsSet(&frameDelims[0], &frameDelims[sizeof(frameDelims)/sizeof(frameDelims[0])]);
}

bool IsDrawCall(Entrypoint entryp) {
    return call_sets::drawCallSet.find(entryp) != call_sets::drawCallSet.end();
}

bool IsFrameDelimiter(Entrypoint entryp) {
    return call_sets::frameDelimsSet.find(entryp) != call_sets::frameDelimsSet.end();
}
