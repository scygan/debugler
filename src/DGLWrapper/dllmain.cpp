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
#include "ipc.h"
#include "globalstate.h"
#include "exechook.h"
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

    ExecHookInitialize();

    //Notify process skipper (for newly executed processes).
    getIPC()->newProcessNotify();

    APILoader& apiLoader = EarlyGlobalState::getApiLoader();

    // load system GL libraries (& initialize entrypoint tables)

    if (getIPC()->getDebuggerMode() == DGLIPC::DebuggerMode::EGL) {

        apiLoader.loadLibrary(LIBRARY_EGL);
        // GL library loading is deferred - we don't know which library to load
        // now.
    } else {
#ifdef _WIN32
        apiLoader.loadLibrary(LIBRARY_WGL);
        apiLoader.loadLibrary(LIBRARY_WINGDI);
#else
        apiLoader.loadLibrary(LIBRARY_GLX);
#endif
        apiLoader.loadLibrary(LIBRARY_GL);
    }
}

/**
 * DGLwrapper routine called on library unload
 */
void TearDown() { GlobalState::reset(); }

#ifndef _WIN32

void __attribute__((constructor)) DGLWrapperLoad(void) { Initialize(); }

void __attribute__((destructor)) DGLWrapperUnload(void) { TearDown(); }
#else

#include <windows.h>

#if DGL_HAVE_WA(ARM_MALI_EMU_LOADERTHREAD_KEEP)

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

#if DGL_HAVE_WA(ARM_MALI_EMU_LOADERTHREAD_KEEP)
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
#if DGL_HAVE_WA(ARM_MALI_EMU_LOADERTHREAD_KEEP)
            g_ThreadWatcher.onAttachThread();
#endif
            break;
        case DLL_THREAD_DETACH:
#if DGL_HAVE_WA(ARM_MALI_EMU_LOADERTHREAD_KEEP)
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
