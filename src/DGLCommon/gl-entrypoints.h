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

#ifndef _GL_ENTRYPOINTS_H
#define _GL_ENTRYPOINTS_H

#include <DGLCommon/gl-types.h>

#define FUNC_LIST_ELEM_SUPPORTED(name, type, library, retVal, params) name##_Call,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library, retVal, params) \
    FUNC_LIST_ELEM_SUPPORTED(name, type, library, retVal, params)
enum Entrypoints {
#include <functionList.inl>
    Entrypoints_NUM
};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

#define NUM_ENTRYPOINTS Entrypoints_NUM
#define NO_ENTRYPOINT Entrypoints_NUM

typedef int Entrypoint;

const char* GetEntryPointName(Entrypoint entryp);
Entrypoint GetEntryPointEnum(const char* name);


class GLParamTypeMetadata {
public:
    enum class BaseType {
        Enum, 
        Bitfield, 
        Value
    };

    GLParamTypeMetadata():m_BaseType(BaseType::Value), m_EnumGroup(GLEnumGroup::None) {}
    GLParamTypeMetadata(BaseType baseType, GLEnumGroup enumGroup):m_BaseType(baseType), m_EnumGroup(enumGroup) {}
    
    BaseType m_BaseType;
    GLEnumGroup m_EnumGroup;    
};

const GLParamTypeMetadata GetEntryPointGLParamTypeMetadata(Entrypoint entryp, int param);
const GLParamTypeMetadata GetEntryPointRetvalMetadata(Entrypoint entryp);

bool IsDrawCall(Entrypoint);
bool IsFrameDelimiter(Entrypoint);

#endif