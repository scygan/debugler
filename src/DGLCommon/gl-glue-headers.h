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

#ifndef GL_GLUE_HEADERS_H
#define GL_GLUE_HEADERS_H

#ifdef _WIN32
    #include <windows.h>

//these two are defined into *A or *W functions. remove this ifdef.
#undef wglUseFontBitmaps
#undef wglUseFontOutlines

#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef _WIN32
#include <GL/wglext.h>
#else
#ifndef __ANDROID__
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#endif

#endif
