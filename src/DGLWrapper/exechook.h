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

#ifdef _WIN32

#include <windows.h>
#include <boost/thread/recursive_mutex.hpp>

/**
 * Static class implementing hooking of CreateProcess (and similar) calls.
 *
 * Hooks are performed to install DGLWrapper to all child processes, so
 * process skipping can work
 * 
 * CreateProcessInternalW is hooked - this is the kernel32.dll implementation
 * common for all CreateProcesA/W/ExA/Exw/other calls.
 */
class ExecHook {
   public:
    /**
     * Hook for CreateProcessInternalW
     *
     * This hook intercepts all CreateProcess calls.
     * Instead just running the process it is started with main thread
     *suspended.
     * In the next step DGLWrapper is injected, than all threads run.
     */
    static BOOL CreateProcessInternalW(
            HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
            LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
            BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir,
            LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi, PHANDLE NewToken);

    /**
     * Call uhooked version of CreateProcessInternalW
     */
    static BOOL real_CreateProcessInternalW(
            HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
            LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
            BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir,
            LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi, PHANDLE NewToken);

    /**
     * Init function.
     *
     * Should be called from loader thread, before use
     */
    static void initialize();

   private:
    /**
     * No CTOR - this is a static class
     */
    ExecHook();

    /**
     * Typedef for CreateProcessInternalW function pointer
     */
    typedef BOOL(WINAPI* CreateProcessInternalW_Type)(
            HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
            LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
            BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir,
            LPSTARTUPINFOW si, LPPROCESS_INFORMATION pi, PHANDLE NewToken);

    /**
     * Pointer to non-hooked, real CreateProcess function
     */
    static CreateProcessInternalW_Type s_real_CreateProcessInternalW;

    /**
     * Mutex guarding the hook code
     */
    static boost::recursive_mutex s_mutex;
};

#endif