#ifndef GL_HEADERS_H
#define GL_HEADERS_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <codegen/input/egl.h>
#include <codegen/input/eglext.h>
#include <GL/gl.h>
#include <codegen/input/glext.h>

#ifdef _WIN32
#include <codegen/input/wglext.h>
#endif

#endif
