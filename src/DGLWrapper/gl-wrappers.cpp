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

#define DGL_WRAPPERS_INSIDE
#include "gl-wrappers.h"

#include <cassert>
#include "pointers.h"
#include "actions.h"

extern "C" {
#include "codegen/wrappers.inl"
}


#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) (FUNC_PTR)&name##_Wrapper,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) NULL,
FUNC_PTR wrapperPtrs[] = {
    #include "codegen/functionList.inl"
    NULL
};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED


FUNC_PTR getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}

extern "C" {
#include "codegen/exporters.inl"
#ifndef __ANDROID__
    //on Linuxes ABI is not really expected and all EXT symbols are exported
    //It does not hurt Windows, also.
    #include "codegen/exporters-ext.inl"
#endif
}
