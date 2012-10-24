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

    g_Controller = boost::make_shared<DebugController>();
    g_Controller->connect(boost::make_shared<dglnet::Server>(5555, g_Controller.get()));
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 ) {
	switch (ul_reason_for_call) {
	    case DLL_PROCESS_ATTACH:
            Initialize();
	    case DLL_THREAD_ATTACH:
	    case DLL_THREAD_DETACH:
	    case DLL_PROCESS_DETACH:
		    break;
	}
	return TRUE;
}

