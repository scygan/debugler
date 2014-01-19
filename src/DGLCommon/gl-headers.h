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
#if !defined(WINGDIAPI) || !defined(APIENTRY)
#define TEMPORARY_APIENTRY_TO_AVOID_WINDOWS_H
#endif
#endif

#if !defined(_WIN32) || defined(TEMPORARY_APIENTRY_TO_AVOID_WINDOWS_H)
#include <KHR/khrplatform.h>
#define WINGDIAPI KHRONOS_APICALL
#define APIENTRY KHRONOS_APIENTRY
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#undef __gl_h_
#include <GLES/gl.h>
#undef __glext_h_
#undef GL_OES_fixed_point
#include <GLES/glext.h>

#undef GL_EXT_separate_shader_objects
#undef GL_ACTIVE_PROGRAM_EXT
#undef GL_KHR_debug

//#undef GL_EXT_draw_buffers
#include <GLES2/gl2ext.h>
#ifdef TEMPORARY_APIENTRY_TO_AVOID_WINDOWS_H
#undef WINGDIAPI
#undef APIENTRY
#endif


#ifdef _WIN32
//these two are defined into *A or *W functions. remove this ifdef.
#undef wglUseFontBitmaps
#undef wglUseFontOutlines

#endif

#endif
