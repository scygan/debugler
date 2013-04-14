// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "tracer.h"
#include "DGLWrapper.h"
#include <boost/make_shared.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

#include <DGLCommon/os.h>

    
/**
 * DGLwrapper routine called on library load
 */
 void Initialize(void) {
    
    std::string dgl_mode = Os::getEnv("dgl_mode");

    //load system GL libraries (& initialize entrypoint tables)

    if (dgl_mode == "egl" ) {
		g_ApiLoader.loadLibrary(LIBRARY_EGL);
		//GL library loading is deferred - we don't know which library to load now.
    } else {
#ifdef _WIN32
        g_ApiLoader.loadLibrary(LIBRARY_WGL);
#else
        g_ApiLoader.loadLibrary(LIBRARY_GLX);
#endif
        g_ApiLoader.loadLibrary(LIBRARY_GL);
    }

    //set default tracer for all entrypoints (std debugging routines)
    SetAllTracers<DefaultTracer>();

    //setup additional, special tracers for some choosed entrypoints 
    //for more specific routines 

    TracerBase::SetNext<GLGetErrorTracer>(glGetError_Call);
    TracerBase::SetNext<GetProcAddressTracer>(wglGetProcAddress_Call);

#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
	TracerBase::SetNext<SurfaceTracer>(eglCreateWindowSurface_Call);
	TracerBase::SetNext<SurfaceTracer>(eglCreatePixmapSurface_Call);
	TracerBase::SetNext<SurfaceTracer>(eglCreatePbufferSurface_Call);
#endif    

    TracerBase::SetNext<ContextTracer>(wglCreateContext_Call);
    TracerBase::SetNext<ContextTracer>(wglCreateContextAttribsARB_Call);
    TracerBase::SetNext<ContextTracer>(wglMakeCurrent_Call);
    TracerBase::SetNext<ContextTracer>(wglDeleteContext_Call);

    TracerBase::SetNext<ContextTracer>(glXCreateContext_Call);
    TracerBase::SetNext<ContextTracer>(glXCreateNewContext_Call);

//    TracerBase::SetNext<ContextTracer>(glXCreateContextAttribsARB_Call); //TODO: no glxext .yet
    TracerBase::SetNext<ContextTracer>(glXMakeCurrent_Call);
    TracerBase::SetNext<ContextTracer>(glXDestroyContext_Call);

    TracerBase::SetNext<ContextTracer>(eglCreateContext_Call);
    TracerBase::SetNext<ContextTracer>(eglMakeCurrent_Call);
    TracerBase::SetNext<ContextTracer>(eglDestroyContext_Call);
    TracerBase::SetNext<ContextTracer>(eglReleaseThread_Call);
	TracerBase::SetNext<ContextTracer>(eglBindAPI_Call);

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

#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
class ThreadWatcher {
public:
    ThreadWatcher():m_ThreadCount(0) {}

    void addThread() {
        boost::lock_guard<boost::mutex> lock(m_ThreadCountLock);
        m_ThreadCount++;
    }

    void deleteThread() {
        boost::lock_guard<boost::mutex> lock(m_ThreadCountLock);
        m_ThreadCount--;
        if (m_ThreadCount < 1) {
            m_LoaderThreadLock.unlock();
        }
    }

    void lockLoaderThread() {
        m_LoaderThreadLock.lock();
        m_LoaderThreadLock.lock();
    }
private: 
    boost::mutex m_LoaderThreadLock, m_ThreadCountLock;
    int m_ThreadCount;
} g_ThreadWatcher;
#endif

/**
 * DGLwrapper routine called just after DLLinjection
 */
extern "C" DGLWRAPPER_API void LoaderThread() {
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
    //this is called from remotely created thread started right after dll injection

    //Workaround for ARM Mali OpenGL ES wrapper on Windows: 
    //do not exit remote loader thread before app tear down
    //  Normally we would just return from this (empty) function causing loader
    //  thread to exit (leaving app in suspended state - no user threads). 
    //  This would also cause DLL_THREAD_DETACH on all recently loaded DLLs. Unfortunately
    //  this causes CreateWindowEx() to fail later in eglInitialize(), propably because
    //  RegisterClass was called in this thread (sic!) from DLLMain.
    //Fix: lock this thread until application finishes

    //tell the loader we are done, so it can resume application
    std::string remoteThreadSemaphoreStr = Os::getEnv("dgl_remote_thread_semaphore");
    boost::interprocess::named_semaphore remoteThreadSemaphore(boost::interprocess::open_only, remoteThreadSemaphoreStr.c_str());
    remoteThreadSemaphore.post();

    
    //wait for application exit (all threads but this exit);
    g_ThreadWatcher.lockLoaderThread();
#endif
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

/**
 * Main entrypoint of DGLwrapper library
 */
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
#ifdef USE_DETOURS
            if (DetourIsHelperProcess()) {
                return TRUE;
            }
#endif
            Os::setCurrentModuleHandle(hModule);
            Initialize();
            break;
        case DLL_THREAD_ATTACH:
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
            g_ThreadWatcher.addThread();
#endif            
            break;
        case DLL_THREAD_DETACH:
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
            g_ThreadWatcher.deleteThread();
#endif
            break;
        case DLL_PROCESS_DETACH:
            TearDown();
            break;
    }
    return TRUE;
}
#endif
