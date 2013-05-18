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


#include <DGLCommon/gl-headers.h>
#include <DGLCommon/gl-types.h>

#include "codegen/nonExtTypedefs.inl"

//POINTER_TYPE(X) returns type of function pointer for entrypoint X. The actual definitions are generated from codegen output
//For entrypoints unsupported on given platform bare void* is returned. 
#define POINTER_TYPE(X) X##_Type

#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) typedef type POINTER_TYPE(name);
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) typedef void* POINTER_TYPE(name);
#include "codegen/functionList.inl"
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

//DIRECT_CALL(X) can be used to directly call entrypoint X, like DIRECT_CALL(glEnable)(GL_BLEND).
#define DIRECT_CALL(X) (*(POINTER_TYPE(X))POINTER(X))
#define POINTER(X) g_DirectPointers[X##_Call].ptr

//DIRECT_CALL_CHECKED(X) can be used call entrypoint X with NULL-checking, like DIRECT_CALL_CHK(glEnable)(GL_BLEND).
//will throw exception on NULL
#define DIRECT_CALL_CHK(X) (*(POINTER_TYPE(X))POINTER_CHECKED(X))
#define POINTER_CHECKED(X) g_ApiLoader.ensurePointer(X##_Call)


struct LoadedPointer {
    void* ptr;
    int libraryMask;
};

extern LoadedPointer g_DirectPointers[Entrypoints_NUM];


