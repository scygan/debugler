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

// The following ifdef block is the standard way of creating macros which make
// exporting
// from a DLL simpler. All files within this DLL are compiled with the
// GLWRAPPER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any
// project
// that uses this DLL. This way any other project whose source files include
// this file see
// GLWRAPPER_API functions as being imported from a DLL, whereas this DLL sees
// symbols
// defined with this macro as being exported.

#ifdef _WIN32
#define DGLWRAPPER_API __declspec(dllexport)
#else
#define DGLWRAPPER_API
#endif

// some auto-configuration for DGLwrapper:

// library coverage (loaded and wrapped):
#define HAVE_LIBRARY_GL
#define HAVE_LIBRARY_GL_EXT
#define HAVE_LIBRARY_EGL
#define HAVE_LIBRARY_EGL_EXT
#define HAVE_LIBRARY_ES1
#define HAVE_LIBRARY_ES1_EXT
#define HAVE_LIBRARY_ES2
#define HAVE_LIBRARY_ES2_EXT
#define HAVE_LIBRARY_ES3
#ifdef _WIN32
#define HAVE_LIBRARY_WGL
#define HAVE_LIBRARY_WGL_EXT
#define HAVE_LIBRARY_WINGDI
#else
#ifndef __ANDROID__
#define HAVE_LIBRARY_GLX
#define HAVE_LIBRARY_GLX_EXT
#endif
#endif

// binary interception method
#ifdef _WIN32
//    #ifdef _WIN64
#define USE_MHOOK
//    #else
//        #define USE_DETOURS
//    #endif
#endif

#include <DGLCommon/wa.h>
