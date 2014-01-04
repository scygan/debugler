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

#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include <DGLCommon/gl-headers.h>
#include <string>

// Some internal types used for message fields, gl-state tracing etc.. (not used
// for gl call params store!).
typedef uint64_t opaque_id_t;    // void*, contexts, configs..
typedef uint64_t gl_t;           // gl enums (including 64bit)
typedef int32_t value_t;         // any gl values (GLints)

typedef int (*FUNC_PTR)();

#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) name##_Call,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) \
    FUNC_LIST_ELEM_SUPPORTED(name, type, library)
enum Entrypoints {
#include <codegen/functionList.inl>
    Entrypoints_NUM
};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

#define NUM_ENTRYPOINTS Entrypoints_NUM
#define NO_ENTRYPOINT Entrypoints_NUM

typedef int Entrypoint;

const char* GetEntryPointName(Entrypoint entryp);
Entrypoint GetEntryPointEnum(const char* name);
std::string GetGLEnumName(gl_t glEnum);

std::string GetShaderStageName(gl_t glEnum);
std::string GetTextureTargetName(gl_t glEnum);

bool IsDrawCall(Entrypoint);
bool IsFrameDelimiter(Entrypoint);

#endif
