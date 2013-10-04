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


#ifndef DGLGUI_H
#define DGLGUI_H

#include<cassert>

#ifdef NDEBUG
#define CONNASSERT(x) x
#else
#define CONNASSERT(x) assert(x)

#ifdef HAVE_VLD
#include <vld.h>
#endif

#endif

#endif
