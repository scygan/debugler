#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <GL/gl.h>

enum Entrypoints {
    #include "codegen/output/functionList.inl"
    LAST_Call
};

#define NUM_ENTRYPOINTS LAST_Call

typedef int Entrypoint;

#endif