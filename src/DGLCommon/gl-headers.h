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
#endif

#endif
