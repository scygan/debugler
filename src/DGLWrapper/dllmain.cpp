// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "tracer.h"

#include <boost/make_shared.hpp>

#include<windows.h>

void Initialize() {
    LoadOpenGLLibrary();

    SetAllTracers<DefaultTracer>();
    TracerBase::SetNext<GetProcAddressTracer>(wglGetProcAddress_Call);
    TracerBase::SetNext<ContextTracer>(wglCreateContext_Call);
    TracerBase::SetNext<ContextTracer>(wglMakeCurrent_Call);
    TracerBase::SetNext<ContextTracer>(wglDeleteContext_Call);

    TracerBase::SetNext<TextureTracer>(glGenTextures_Call);
    TracerBase::SetNext<TextureTracer>(glDeleteTextures_Call);
    TracerBase::SetNext<TextureTracer>(glBindTexture_Call);

    TracerBase::SetNext<BufferTracer>(glGenBuffers_Call);
    TracerBase::SetNext<BufferTracer>(glDeleteBuffers_Call);
    TracerBase::SetNext<BufferTracer>(glBindBuffer_Call);

    TracerBase::SetNext<ProgramTracer>(glCreateProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glDeleteProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glUseProgram_Call);

    g_Controller = boost::make_shared<DebugController>();
    boost::shared_ptr<dglnet::Server> srv = boost::make_shared<dglnet::Server>(5555, g_Controller.get());
    srv->accept();
    g_Controller->connect(srv);
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
            Initialize(); break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            TearDown();
            break;
    }
    return TRUE;
}

