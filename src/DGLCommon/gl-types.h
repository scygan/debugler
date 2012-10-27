#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include <DGLcommon/gl-headers.h>


#define FUNCTION_LIST_ELEMENT(name, type) name##_Call,
enum Entrypoints {
    #include "../../dump/codegen/functionList.inl"
    Entrypoints_NUM

};
#undef FUNCTION_LIST_ELEMENT

#define NUM_ENTRYPOINTS Entrypoints_NUM

typedef int Entrypoint;


char* GetEntryPointName(Entrypoint entrp);
Entrypoint GetEntryPointEnum(const char* name);

#endif


