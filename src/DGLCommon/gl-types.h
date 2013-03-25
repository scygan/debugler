#ifndef _GL_TYPES_H
#define _GL_TYPES_H

#include<DGLCommon/gl-headers.h>
#include<string>

//Some internal types used for message fields, gl-state tracing etc.. (not used for gl call params store!).
typedef uint64_t opaque_id_t; //void*, contexts, configs..
typedef uint64_t gl_t;        //gl enums (including 64bit)
typedef int32_t  value_t;     //any gl values (GLints)


#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) name##_Call,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) FUNC_LIST_ELEM_SUPPORTED(name, type, library)
enum Entrypoints {
    #include "codegen/functionList.inl"
    Entrypoints_NUM

};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED

#define NUM_ENTRYPOINTS Entrypoints_NUM
#define NO_ENTRYPOINT Entrypoints_NUM

typedef int Entrypoint;


const char* GetEntryPointName(Entrypoint entryp);
Entrypoint GetEntryPointEnum(const char* name);
std::string GetGLEnumName(gl_t glEnum);

std::string GetShaderStageName(gl_t glEnum);
std::string GetTextureTargetName(gl_t glEnum);


bool IsDrawCall(Entrypoint);
bool IsFrameDelimiter(Entrypoint);

#endif


