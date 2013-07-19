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
#include <windows.h>
#else
#define WINGDIAPI KHRONOS_APICALL
#define APIENTRY KHRONOS_APIENTRY
#endif


//Only headers needed to compile debugger + some corrections
#include <codegen/input/egl.h>
#include <codegen/input/eglext.h>
#include <codegen/input/GL.h>
#include <codegen/input/glext.h>
#undef __gl_h_
#include <codegen/input/GLESv1/gl.h>
#undef __glext_h_
#include <codegen/input/GLESv1/glext.h>

#undef GL_EXT_separate_shader_objects
#undef GL_KHR_debug
#undef GL_EXT_draw_buffers
#include <codegen/input/gl2ext.h>
//fix some errors in gl2ext:
#define PFNGLBLITFRAMEBUFFERNVPROC                PFNBLITFRAMEBUFFERNVPROC
#define PFNGLDRAWARRAYSINSTANCEDNVPROC            PFNDRAWARRAYSINSTANCEDNVPROC
#define PFNGLDRAWELEMENTSINSTANCEDNVPROC          PFNDRAWELEMENTSINSTANCEDNVPROC
#define PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC PFNRENDERBUFFERSTORAGEMULTISAMPLENVPROC
#define PFNGLVERTEXATTRIBDIVISORNVPROC            PFNVERTEXATTRIBDIVISORNVPROC
#define PFNGLFRAMEBUFFERTEXTURE3DOESPROC          PFNGLFRAMEBUFFERTEXTURE3DOES

#ifdef _WIN32
#include <codegen/input/wglext.h>
#else
#ifndef __ANDROID__
#include <codegen/input/glx.h>
#include <codegen/input/glxext.h>
#endif
#endif

#endif
