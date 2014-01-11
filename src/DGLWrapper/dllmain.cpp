/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// dllmain.cpp : Defines the entry point for the DLL application.

#include "api-loader.h"
#include "debugger.h"
#include "actions.h"
#include "ipc.h"
#include "DGLWrapper.h"
#include <boost/make_shared.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

#include <DGLCommon/os.h>
#include <atomic>
#include <thread>
#include <condition_variable>

DGLIPC* getIPC() {
    static std::shared_ptr<DGLIPC> s_IPC;

    if (!s_IPC.get()) {
        s_IPC = DGLIPC::CreateFromUUID(Os::getEnv("dgl_uuid"));
    }

    return s_IPC.get();
}

/**
 * DGLwrapper routine called on library load
 */
void Initialize(void) {

    // load system GL libraries (& initialize entrypoint tables)

    if (getIPC()->getDebuggerMode() == DGLIPC::DebuggerMode::EGL) {
        g_ApiLoader.loadLibrary(LIBRARY_EGL);
        // GL library loading is deferred - we don't know which library to load
        // now.
    } else {
#ifdef _WIN32
        g_ApiLoader.loadLibrary(LIBRARY_WGL);
#else
        g_ApiLoader.loadLibrary(LIBRARY_GLX);
#endif
        g_ApiLoader.loadLibrary(LIBRARY_GL);
    }

    // set default action for all entrypoints (std debugging routines)
    SetAllActions<DefaultAction>();

    // setup additional, special actions for some choosed entrypoints
    // for more specific routines

    ActionBase::SetNext<GLGetErrorAction>(glGetError_Call);
    ActionBase::SetNext<GetProcAddressAction>(wglGetProcAddress_Call);
    ActionBase::SetNext<GetProcAddressAction>(glXGetProcAddress_Call);
    ActionBase::SetNext<GetProcAddressAction>(glXGetProcAddressARB_Call);
    ActionBase::SetNext<GetProcAddressAction>(eglGetProcAddress_Call);

#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
    ActionBase::SetNext<SurfaceAction>(eglCreateWindowSurface_Call);
    ActionBase::SetNext<SurfaceAction>(eglCreatePixmapSurface_Call);
    ActionBase::SetNext<SurfaceAction>(eglCreatePbufferSurface_Call);
#endif

    ActionBase::SetNext<ContextAction>(wglCreateContext_Call);
    ActionBase::SetNext<ContextAction>(wglCreateContextAttribsARB_Call);
    ActionBase::SetNext<ContextAction>(wglMakeCurrent_Call);
    ActionBase::SetNext<ContextAction>(wglDeleteContext_Call);

    ActionBase::SetNext<ContextAction>(glXCreateContext_Call);
    ActionBase::SetNext<ContextAction>(glXCreateNewContext_Call);
    ActionBase::SetNext<ContextAction>(glXCreateContextAttribsARB_Call);

    ActionBase::SetNext<ContextAction>(glXMakeContextCurrent_Call);
    ActionBase::SetNext<ContextAction>(glXMakeCurrent_Call);
    ActionBase::SetNext<ContextAction>(glXDestroyContext_Call);

    ActionBase::SetNext<ContextAction>(eglCreateContext_Call);
    ActionBase::SetNext<ContextAction>(eglMakeCurrent_Call);
    ActionBase::SetNext<ContextAction>(eglDestroyContext_Call);
    ActionBase::SetNext<ContextAction>(eglReleaseThread_Call);
    ActionBase::SetNext<ContextAction>(eglBindAPI_Call);

    ActionBase::SetNext<DebugContextAction>(wglCreateContext_Call);
    ActionBase::SetNext<DebugContextAction>(wglCreateContextAttribsARB_Call);
    ActionBase::SetNext<DebugContextAction>(glXCreateContext_Call);
    ActionBase::SetNext<DebugContextAction>(glXCreateNewContext_Call);
    ActionBase::SetNext<DebugContextAction>(glXCreateContextAttribsARB_Call);

    ActionBase::SetNext<TextureAction>(glGenTextures_Call);
    ActionBase::SetNext<TextureAction>(glGenTexturesEXT_Call);
    ActionBase::SetNext<TextureAction>(glDeleteTextures_Call);
    ActionBase::SetNext<TextureAction>(glDeleteTexturesEXT_Call);
    ActionBase::SetNext<TextureAction>(glBindTexture_Call);
    ActionBase::SetNext<TextureAction>(glBindTextureEXT_Call);

    ActionBase::SetNext<TextureFormatAction>(glTexImage1D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage2D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage2DMultisample_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage3D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage3DEXT_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage3DOES_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexImage3DMultisample_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage1D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage2D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage2DMultisample_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage3D_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage3DMultisample_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage1DEXT_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage2DEXT_Call);
    ActionBase::SetNext<TextureFormatAction>(glTexStorage3DEXT_Call);

    ActionBase::SetNext<BufferAction>(glGenBuffers_Call);
    ActionBase::SetNext<BufferAction>(glGenBuffersARB_Call);
    ActionBase::SetNext<BufferAction>(glDeleteBuffers_Call);
    ActionBase::SetNext<BufferAction>(glDeleteBuffersARB_Call);
    ActionBase::SetNext<BufferAction>(glBindBuffer_Call);
    ActionBase::SetNext<BufferAction>(glBindBufferARB_Call);

    ActionBase::SetNext<FBOAction>(glGenFramebuffers_Call);
    ActionBase::SetNext<FBOAction>(glGenFramebuffersEXT_Call);
    ActionBase::SetNext<FBOAction>(glDeleteFramebuffers_Call);
    ActionBase::SetNext<FBOAction>(glDeleteFramebuffersEXT_Call);
    ActionBase::SetNext<FBOAction>(glBindFramebuffer_Call);
    ActionBase::SetNext<FBOAction>(glBindFramebufferEXT_Call);

    ActionBase::SetNext<ProgramAction>(glCreateProgram_Call);
    ActionBase::SetNext<ProgramAction>(glCreateProgramObjectARB_Call);
    ActionBase::SetNext<ProgramAction>(glDeleteProgram_Call);
    ActionBase::SetNext<ProgramAction>(glDeleteObjectARB_Call);
    ActionBase::SetNext<ProgramAction>(glUseProgram_Call);
    ActionBase::SetNext<ProgramAction>(glUseProgramObjectARB_Call);
    ActionBase::SetNext<ProgramAction>(glLinkProgram_Call);
    ActionBase::SetNext<ProgramAction>(glLinkProgramARB_Call);

    ActionBase::SetNext<ShaderAction>(glCreateShader_Call);
    ActionBase::SetNext<ShaderAction>(glCreateShaderObjectARB_Call);
    ActionBase::SetNext<ShaderAction>(glDeleteShader_Call);
    ActionBase::SetNext<ShaderAction>(glDeleteObjectARB_Call);
    ActionBase::SetNext<ShaderAction>(glCompileShader_Call);
    ActionBase::SetNext<ShaderAction>(glCompileShaderARB_Call);
    ActionBase::SetNext<ShaderAction>(glAttachObjectARB_Call);
    ActionBase::SetNext<ShaderAction>(glAttachShader_Call);
    ActionBase::SetNext<ShaderAction>(glDetachObjectARB_Call);
    ActionBase::SetNext<ShaderAction>(glDetachShader_Call);
    ActionBase::SetNext<ShaderAction>(glShaderSource_Call);
    ActionBase::SetNext<ShaderAction>(glShaderSourceARB_Call);

    ActionBase::SetNext<ImmediateModeAction>(glBegin_Call);
    ActionBase::SetNext<ImmediateModeAction>(glEnd_Call);

    ActionBase::SetNext<DebugOutputCallback>(glDebugMessageCallback_Call);
    ActionBase::SetNext<DebugOutputCallback>(glDebugMessageCallbackARB_Call);
}

/**
 * DGLwrapper routine called on library unload
 */
void TearDown() { _g_Controller.reset(); }

#ifndef _WIN32
void __attribute__((constructor)) DGLWrapperLoad(void) { Initialize(); }

void __attribute__((destructor)) DGLWrapperUnload(void) { TearDown(); }
#else

#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
#include <windows.h>
class ThreadWatcher {
   public:
    ThreadWatcher() : m_ThreadCount() {
        m_ThreadCount = 0;

        m_NativeSemaphore = CreateSemaphore(NULL, 0, 0xffff, NULL);
        if (!m_NativeSemaphore) {
            Os::fatal(
                    "Cannot create loader semaphore (CreateSemaphore failed)");
        }
    }

    ~ThreadWatcher() {
        if (!CloseHandle(m_NativeSemaphore)) {
            Os::fatal("Cannot close loader semaphore (CloseHandle failed)");
        }
    }

    void onAttachThread() { m_ThreadCount++; }

    void onDettachThread() {
        if (std::atomic_fetch_sub(&m_ThreadCount, 1) <= 1) {
            unlockLoaderThread();
        }
    }

    void unlockLoaderThread() {
        if (!ReleaseSemaphore(m_NativeSemaphore, 1, NULL)) {
            Os::fatal("Cannot unlock loader thread (ReleaseSemaphore failed)");
        }
    }

    void lockLoaderThread() {
        if (WaitForSingleObject(m_NativeSemaphore, INFINITE) != WAIT_OBJECT_0) {
            Os::fatal(
                    "Cannot loack loader thread (WaitForSingleObject failed)");
        }
    }

   private:
    HANDLE m_NativeSemaphore;
    std::atomic_int m_ThreadCount;
} g_ThreadWatcher;
#endif

/**
 * DGLwrapper routine called just after DLLinjection
 */
extern "C" DGLWRAPPER_API void LoaderThread() {

    Initialize();

#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
    // this is called from remotely created thread started right after dll
    // injection

    // Workaround for ARM Mali OpenGL ES wrapper on Windows:
    // do not exit remote loader thread before app tear down
    //  Normally we would just return from this (empty) function causing loader
    //  thread to exit (leaving app in suspended state - no user threads).
    //  This would also cause DLL_THREAD_DETACH on all recently loaded DLLs.
    // Unfortunately
    //  this causes CreateWindowEx() to fail later in eglInitialize(), propably
    // because
    //  RegisterClass was called in this thread (sic!) from DLLMain.
    // Fix: lock this thread until application finishes

    // tell the loader we are done, so it can resume application
    getIPC()->postRemoteThreadSemaphore();

    // wait for application exit (all threads but this exit);
    g_ThreadWatcher.lockLoaderThread();
#endif
}
/**
 * Main entrypoint of DGLwrapper library
 */
BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/
        ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
#ifdef USE_DETOURS
            if (DetourIsHelperProcess()) {
                return TRUE;
            }
#endif
            Os::setCurrentModuleHandle(hModule);
            break;
        case DLL_THREAD_ATTACH:
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
            g_ThreadWatcher.onAttachThread();
#endif
            break;
        case DLL_THREAD_DETACH:
#ifdef WA_ARM_MALI_EMU_LOADERTHREAD_KEEP
            g_ThreadWatcher.onDettachThread();
#endif
            break;
        case DLL_PROCESS_DETACH:
            TearDown();
            break;
    }
    return TRUE;
}
#endif
