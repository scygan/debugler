#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include <DGLcommon/gl-headers.h>


#define FUNCTION_LIST_ELEMENT(name, type) name##_Call,
enum Entrypoints {
    #include "../../dump/codegen/functionList.inl"
    LAST_Call

};
#undef FUNCTION_LIST_ELEMENT

#define NUM_ENTRYPOINTS LAST_Call

typedef int Entrypoint;


extern char* GetEntryPointName(Entrypoint entrp);

#endif


