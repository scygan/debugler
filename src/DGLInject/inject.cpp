/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include "inject.h"

#ifdef _WIN32
#include <windows.h>
#include "CompleteInject/CompleteInject.h"
#define snprintf _snprintf
#endif

#include <DGLCommon/wa.h>
#include <DGLCommon/os.h>
#include <DGLCommon/ipc.h>


#ifdef _WIN32
bool isProcess64Bit(HANDLE hProcess) {
    // get IsWow64Process function
    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = fnIsWow64Process =
        (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    BOOL isWow;

    // check if windows is 64-bit
    bool isWindows64bit = false;

#ifdef _WIN64
    // if we are a 64 bit app, windows is definitely 64 bit
    isWindows64bit = true;
#else
    // we are 32-bit application. Check if we are in Wow64 mode
    if (fnIsWow64Process && fnIsWow64Process(GetCurrentProcess(), &isWow) &&
        isWow) {
            // if we are in Wow64 mode, windows is 64 bit
            isWindows64bit = true;
    }
#endif

    // check if process is 64 bit
    if (isWindows64bit && fnIsWow64Process &&
        fnIsWow64Process(hProcess, &isWow) && !isWow) {
            // windows is 64 bit, process is not in Wow64 mode, so process is 64
            // bit. This implies usage of 64 bit wrapper
            return true;
    }
    return false;
}
#endif


DGLInject::DGLInject(const std::string& wrapperPath):m_WrapperPath(wrapperPath) {}


void DGLInject::injectPostFork() {
#ifndef _WIN32
    Os::setEnv("LD_PRELOAD", m_WrapperPath.c_str());
#endif
}

void DGLInject::injectPostExec(DGLIPC& ipc, native_process_handle_t processHandle, native_process_handle_t mainThreadHandle) {
#ifdef _WIN32

#ifdef _WIN64
    if (!isProcess64Bit(processHandle)) {
        throw std::runtime_error(
            "Incompatible loader version: used 64bit, but process is "
            "not "
            "64bit");
    }
#else
    if (isProcess64Bit(processHandle)) {
        throw std::runtime_error(
            "Incompatible loader version: used 32bit, but process is "
            "not "
            "32bit");
    }
#endif

    // inject thread with wrapper library loading code, run through DLLMain and
    // LoaderThread() function
#if DGL_HAVE_WA(ARM_MALI_EMU_LOADERTHREAD_KEEP)
    Inject(processHandle, m_WrapperPath.c_str(), "LoaderThread");
    ipc.waitForRemoteThreadSemaphore();
#else
    HANDLE remoteThread = Inject(processHandle, m_WrapperPath.c_str(), "LoaderThread");
    // wait for loader thread to finish dll inject
    WaitForSingleObject(remoteThread, INFINITE);
    DGL_UNUSED(ipc);
#endif

    // resume process - now user thread is running
    // whole OpenGL should be wrapped by now

    if (ResumeThread(mainThreadHandle) == -1) {
        throw std::runtime_error("Cannot resume process");
    }
#else
    DGL_UNUSED(ipc);
    DGL_UNUSED(processHandle);
    DGL_UNUSED(mainThreadHandle);
#endif
}