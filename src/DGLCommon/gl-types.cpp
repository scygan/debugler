/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "gl-types.h"

#include<cstdio>
#include<map>
#include<set>
#include<sstream>

#include<boost/make_shared.hpp>

namespace lists {
#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) #name,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) FUNC_LIST_ELEM_SUPPORTED(name, type, library)
    const char* g_EntrypointNames[] = {
#include "codegen/functionList.inl"
        "<unknown>"
    };
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

#define ENUM_LIST_ELEMENT(value) {#value, value},
    struct GLEnum {
        const char* name; 
        gl_t value;
    }  g_GLEnums[] = {
#include "codegen/enum.inl"
        { NULL, 0 }
    };
#undef ENUM_LIST_ELEMENT

    struct MapCache {
        MapCache() {

            for (Entrypoint e = 0; e < Entrypoints_NUM; e++) {
                entryPointNameToEnum[GetEntryPointName(e)] = e;
            }

            int i = 0; 
            while (g_GLEnums[i].name) {
                if (EnumGLToName.find(g_GLEnums[i].value) == EnumGLToName.end())
                    //do not overwrite any enum that already exist in the map:
                    //the first come GLenums are "older" - were defined earlier in OGL history. 
                    //the duplicates are usually some vendor extensions from the bottom glext.h
                    EnumGLToName[g_GLEnums[i].value] = g_GLEnums[i].name;
                i++;
            }
        }

        static MapCache* get() {
            if (!s_cache.get()) {
                s_cache = boost::make_shared<MapCache>();
            }
            return s_cache.get();
        }

        static boost::shared_ptr<MapCache> s_cache;
        std::map<std::string, Entrypoint> entryPointNameToEnum;
        std::map<gl_t, std::string> EnumGLToName;
    };

   boost::shared_ptr<MapCache> MapCache::s_cache;

}


const char* GetEntryPointName(Entrypoint entryp) {
    return lists::g_EntrypointNames[entryp];
}

Entrypoint GetEntryPointEnum(const char* name) {
    std::map<std::string, Entrypoint>::iterator ret = lists::MapCache::get()->entryPointNameToEnum.find(name);
    if (ret == lists::MapCache::get()->entryPointNameToEnum.end())
        return NO_ENTRYPOINT;
    return ret->second;
}

std::string GetGLEnumName(gl_t glEnum) {
    std::map<gl_t, std::string>::iterator ret = lists::MapCache::get()->EnumGLToName.find(glEnum);
    if (ret == lists::MapCache::get()->EnumGLToName.end()) {
        std::ostringstream tmp; 
        tmp << "0x" << std::hex << glEnum;
        return tmp.str();
    }
    return ret->second;
}

std::string GetShaderStageName(gl_t glEnum) {
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
        case GL_COMPUTE_SHADER:
            return "Compute";
        default:
            return GetGLEnumName(glEnum).c_str();
    }
}

std::string GetTextureTargetName(gl_t glEnum) {
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
         wglSwapMultipleBuffers_Call,
         eglSwapBuffers_Call,
         glXSwapBuffers_Call
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
        glMultiDrawRangeElementArrayAPPLE_Call,
        glDrawTransformFeedback_Call,
        glDrawTransformFeedbackInstanced_Call,
        glDrawTransformFeedbackNV_Call,
        glDrawTransformFeedbackStream_Call,
        glDrawTransformFeedbackStreamInstanced_Call,
        glEnd_Call,
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
