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


#ifndef GL_HEADERS_H
#define GL_HEADERS_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define WINGDIAPI KHRONOS_APICALL
#define APIENTRY KHRONOS_APIENTRY
#endif

#include <codegen/input/egl.h>
#include <codegen/input/eglext.h>
#include <codegen/input/GL.h>
#include <codegen/input/glext.h>

#ifdef _WIN32
#include <codegen/input/wglext.h>
#else
#include <codegen/input/glx.h>
//#include <codegen/input/glxext.h>
#endif

#endif
