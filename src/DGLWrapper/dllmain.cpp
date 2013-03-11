// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "tracer.h"
#include "DGLWrapper.h"
#include <boost/make_shared.hpp>

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

#include <DGLCommon/os.h>

    
/**
 * DGLwrapper routine called on library load
 */
 void Initialize(void) {
    
    //load system GL libraries (& initialize entrypoint tables)
#ifdef _WIN32
    LoadOpenGLLibrary("opengl32.dll", LIBRARY_WGL | LIBRARY_GL);
#else
    LoadOpenGLLibrary("libGL.so.1", LIBRARY_GL);
#endif

    //set default tracer for all entrypoints (std debugging routines)
    SetAllTracers<DefaultTracer>();

    //setup additional, special tracers for some choosed entrypoints 
    //for more specific routines 

    TracerBase::SetNext<GLGetErrorTracer>(glGetError_Call);
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
    TracerBase::SetNext<ProgramTracer>(glLinkProgram_Call);
    TracerBase::SetNext<ProgramTracer>(glLinkProgramARB_Call);

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
}

/**
 * DGLwrapper routine called just after DLLinjection
 */
extern "C" APIENTRY void InitializeThread() {

    //this is called from remotely created thread started right after dll injection

    // empty.
}

/**
 * DGLwrapper routine called on library unload
 */
void TearDown() {
    _g_Controller.reset();
}



#ifndef _WIN32
void __attribute__ ((constructor)) DGLWrapperLoad(void) {
    Initialize();
}

void __attribute__ ((destructor)) DGLWrapperUnload(void) {
    TearDown();
}
#else

bool once = false;

/**
 * Main entrypoint of DGLwrapper library
 */
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            if (once) return TRUE;
            once = true;
            
#ifdef USE_DETOURS
            if (DetourIsHelperProcess()) {
                return TRUE;
            }
#endif
            Os::setCurrentModuleHandle(hModule);
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
#endif
