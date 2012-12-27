#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include <DGLcommon/gl-headers.h>
#include<string>


#define FUNCTION_LIST_ELEMENT(name, type) name##_Call,
enum Entrypoints {
    #include "../../dump/codegen/functionList.inl"
    Entrypoints_NUM

};
#undef FUNCTION_LIST_ELEMENT

#define NUM_ENTRYPOINTS Entrypoints_NUM
#define NO_ENTRYPOINT Entrypoints_NUM

typedef int Entrypoint;


char* GetEntryPointName(Entrypoint entryp);
Entrypoint GetEntryPointEnum(const char* name);
std::string GetGLEnumName(uint64_t glEnum);

const char* GetShaderStageName(uint64_t glEnum);


bool IsDrawCall(Entrypoint);
bool IsFrameDelimiter(Entrypoint);

#endif


