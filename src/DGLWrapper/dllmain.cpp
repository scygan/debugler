// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "tracer.h"

#include <boost/make_shared.hpp>

#include<windows.h>

void Initialize() {
    LoadOpenGLLibrary();

    SetAllTracers<DefaultTracer>();
    SetTracer<GetProcAddressTracer>(wglGetProcAddress_Call);
    SetTracer<ContextTracer>(wglCreateContext_Call);
    SetTracer<ContextTracer>(wglMakeCurrent_Call);
    SetTracer<ContextTracer>(wglDeleteContext_Call);

    SetTracer<TextureTracer>(glGenTextures_Call);
    SetTracer<TextureTracer>(glDeleteTextures_Call);
    SetTracer<TextureTracer>(glBindTexture_Call);

    SetTracer<BufferTracer>(glGenBuffers_Call);
    SetTracer<BufferTracer>(glDeleteBuffers_Call);
    SetTracer<BufferTracer>(glBindBuffer_Call);

    g_Controller = boost::make_shared<DebugController>();
    boost::shared_ptr<dglnet::Server> srv = boost::make_shared<dglnet::Server>(5555, g_Controller.get());
    srv->accept();
    g_Controller->connect(srv);
}

void TearDown() {
    g_Controller->getServer().disconnect();
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

