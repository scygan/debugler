// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "tracer.h"
#include "DGLWrapper.h"
#include <boost/make_shared.hpp>


#include "detours/detours.h"


extern "C" __declspec(dllexport) void InitializeThread() {}
     
     
 void Initialize(void) {
    
    LoadOpenGLLibrary();

    SetAllTracers<DefaultTracer>();
    TracerBase::SetNext<GetProcAddressTracer>(wglGetProcAddress_Call);
    TracerBase::SetNext<ContextTracer>(wglCreateContext_Call);
    TracerBase::SetNext<ContextTracer>(wglMakeCurrent_Call);
    TracerBase::SetNext<ContextTracer>(wglDeleteContext_Call);

    TracerBase::SetNext<DebugContextTracer>(wglCreateContext_Call);
    TracerBase::SetNext<DebugContextTracer>(wglCreateContextAttribsARB_Call);

    TracerBase::SetNext<TextureTracer>(glGenTextures_Call);
    TracerBase::SetNext<TextureTracer>(glGenTexturesEXT_Call);
    TracerBase::SetNext<TextureTracer>(glDeleteTextures_Call);
    TracerBase::SetNext<TextureTracer>(glDeleteTexturesEXT_Call);
    TracerBase::SetNext<TextureTracer>(glBindTexture_Call);
    TracerBase::SetNext<TextureTracer>(glBindTextureEXT_Call);

    TracerBase::SetNext<BufferTracer>(glGenBuffers_Call);
    TracerBase::SetNext<BufferTracer>(glGenBuffersARB_Call);
    TracerBase::SetNext<BufferTracer>(glDeleteBuffers_Call);
    TracerBase::SetNext<BufferTracer>(glDeleteBuffersARB_Call);
    TracerBase::SetNext<BufferTracer>(glBindBuffer_Call);
    TracerBase::SetNext<BufferTracer>(glBindBufferARB_Call);

    TracerBase::SetNext<FBOTracer>(glGenFramebuffers_Call);
    TracerBase::SetNext<FBOTracer>(glGenFramebuffersEXT_Call);
    TracerBase::SetNext<FBOTracer>(glDeleteFramebuffers_Call);
    TracerBase::SetNext<FBOTracer>(glDeleteFramebuffersEXT_Call);
    TracerBase::SetNext<FBOTracer>(glBindFramebuffer_Call);
    TracerBase::SetNext<FBOTracer>(glBindFramebufferEXT_Call);

    TracerBase::SetNext<ProgramTracer>(glCreateProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glCreateProgramObjectARB_Call);
    TracerBase::SetNext<ProgramTracer>(glDeleteProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glDeleteObjectARB_Call);
    TracerBase::SetNext<ProgramTracer>(glUseProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glUseProgramObjectARB_Call);

    TracerBase::SetNext<ShaderTracer>(glCreateShader_Call);
    TracerBase::SetNext<ShaderTracer>(glCreateShaderObjectARB_Call);
    TracerBase::SetNext<ShaderTracer>(glShaderSource_Call);
    TracerBase::SetNext<ShaderTracer>(glShaderSourceARB_Call);
    TracerBase::SetNext<ShaderTracer>(glDeleteShader_Call);
    TracerBase::SetNext<ShaderTracer>(glDeleteObjectARB_Call);
    TracerBase::SetNext<ShaderTracer>(glCompileShader_Call);
    TracerBase::SetNext<ShaderTracer>(glCompileShaderARB_Call);
    TracerBase::SetNext<ShaderTracer>(glAttachObjectARB_Call);
    TracerBase::SetNext<ShaderTracer>(glAttachShader_Call);

    TracerBase::SetNext<ImmediateModeTracer>(glBegin_Call);
    TracerBase::SetNext<ImmediateModeTracer>(glEnd_Call);
    
    int port = 8888;
    /*char* portStr; size_t len;
    if (_dupenv_s(&portStr, &len, "dgl_port") == 0 && portStr && strlen(portStr)) {
        port = atoi(portStr);
    }*/

    g_Controller = boost::make_shared<DGLDebugController>();
    //boost::shared_ptr<dglnet::Server> srv = boost::make_shared<dglnet::Server>(port, g_Controller.get());
    //srv->accept();
    //g_Controller->connect(srv);
    //SetDllDirectoryA("C:\\Users\\Administrator\\Desktop\\debugler\\build\\Debug\\DGLWrapper");
}

void TearDown() {
    g_Controller.reset();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            if (DetourIsHelperProcess()) {
                return TRUE;
            }
            Initialize();
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            TearDown();
            break;
    }
    return TRUE;
}

