#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include<DGLCommon/gl-headers.h>
#include<string>


#define FUNCTION_LIST_ELEMENT(name, type, library) name##_Call,
enum Entrypoints {
    #include "codegen/functionList.inl"
    Entrypoints_NUM

};
#undef FUNCTION_LIST_ELEMENT

#define NUM_ENTRYPOINTS Entrypoints_NUM
#define NO_ENTRYPOINT Entrypoints_NUM

typedef int Entrypoint;


const char* GetEntryPointName(Entrypoint entryp);
Entrypoint GetEntryPointEnum(const char* name);
std::string GetGLEnumName(uint64_t glEnum);

std::string GetShaderStageName(uint64_t glEnum);
std::string GetTextureTargetName(uint64_t glEnum);


bool IsDrawCall(Entrypoint);
bool IsFrameDelimiter(Entrypoint);

#endif


