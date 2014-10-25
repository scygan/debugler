/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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
#include "gl-glue-headers.h"

#include <memory>
#include <cstdio>
#include <map>
#include <set>
#include <sstream>

namespace lists {

struct GLEnumName {
public:
    const char* m_name;
    GLEnumGroup  m_groups[11];
    bool operator<(const GLEnumName& rhs) const {
        return m_name < rhs.m_name;
    }
};

struct EnumMapCache {
    EnumMapCache() {
        for (size_t i = 0; s_CodegenEnumGLToName[i].name.m_name != nullptr; i++) {
            EnumGLToName[s_CodegenEnumGLToName[i].value].insert(&s_CodegenEnumGLToName[i].name);
        }
    }

    static EnumMapCache* get() {
        if (!s_cache.get()) {
            s_cache = std::make_shared<EnumMapCache>();
        }
        return s_cache.get();
    }

    static std::shared_ptr<EnumMapCache> s_cache;
    std::map<gl_t, std::set<GLEnumName*>> EnumGLToName;

    static struct CodegenEnumGLToName {
        //CodegenEnumGLToName(gl_t v, GLEnumName n):value(v), name(n) {}
        gl_t value;
        GLEnumName name;
    } s_CodegenEnumGLToName[];
};



EnumMapCache::CodegenEnumGLToName EnumMapCache::s_CodegenEnumGLToName[] = {

#define ENUM_LIST_ELEMENT(name, value, ...) \
    {(gl_t)value,  { #name, {__VA_ARGS__, GLEnumGroup::NoneGroup}}},
#include "gl_enum_list.inl"
#undef ENUM_LIST_ELEMENT
    {0,  {nullptr, {}}},
};


std::shared_ptr<EnumMapCache> EnumMapCache::s_cache;

} //namespace lists

std::string GetGLEnumName(gl_t glEnum, GLEnumGroup group) {

    std::map<gl_t, std::set<lists::GLEnumName*>>::iterator nameSetPtr =
            lists::EnumMapCache::get()->EnumGLToName.find(glEnum);

    if (nameSetPtr == lists::EnumMapCache::get()->EnumGLToName.end()) {

        //unknown enum, return hex value string.
        std::ostringstream tmp;
        tmp << "0x" << std::hex << glEnum;
        return tmp.str();
    }

    std::set<lists::GLEnumName*>& nameSet = nameSetPtr->second;

    for (std::set<lists::GLEnumName*>::iterator i = nameSet.begin(); 
        i != nameSet.end(); ++i) {

        for (size_t j = 0; (*i)->m_groups[j] != GLEnumGroup::NoneGroup; j++) {
            if ((*i)->m_groups[j] == group) {
                //found name matching requested enum group.
                return (*i)->m_name;
            }
        }
    }

    //requested group does not match any known name of this entrypoint. Return any name.

    return (*nameSet.begin())->m_name;
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
            return GetGLEnumName(glEnum); //no GLenumGroup for shader yet..
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
        case 0:
            return "no target";
        default:
            return GetGLEnumName(glEnum, GLEnumGroup::TextureTarget);
    }
}
