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

#include "exechook.h"
#include "ipc.h"

#include <DGLCommon/os.h>

#ifdef _WIN32
#include <windows.h>
#ifdef USE_MHOOK
#include "mhook/mhook-lib/mhook.h"
#endif
#endif

#include <DGLInject/inject.h>

#ifdef _WIN32
typedef BOOL  (WINAPI* CreateProcessInternalW_Type)(
    HANDLE hToken,
    LPCWSTR AppName,
    LPWSTR CmdLine,
    LPSECURITY_ATTRIBUTES ProcessAttr,
    LPSECURITY_ATTRIBUTES ThreadAttr,
    BOOL bIH,
    DWORD flags,
    LPVOID env,
    LPCWSTR CurrDir,
    LPSTARTUPINFOW si,
    LPPROCESS_INFORMATION pi,
    PHANDLE NewToken);

CreateProcessInternalW_Type origCreateProcessInternalW = nullptr;



BOOL WINAPI CreateProcessInternalW_CALL(
    HANDLE hToken,
    LPCWSTR AppName,
    LPWSTR CmdLine,
    LPSECURITY_ATTRIBUTES ProcessAttr,
    LPSECURITY_ATTRIBUTES ThreadAttr,
    BOOL bIH,
    DWORD flags,
    LPVOID env,
    LPCWSTR CurrDir,
    LPSTARTUPINFOW si,
    LPPROCESS_INFORMATION pi,
    PHANDLE NewToken) {

    DGLIPC* ipc = getIPC();

    DGLInject inject(ipc->getWrapperPath());

    BOOL ret = origCreateProcessInternalW(hToken, AppName, CmdLine, ProcessAttr, ThreadAttr, bIH, flags | CREATE_SUSPENDED, env, CurrDir, si, pi, NewToken);

    if (ret) {
        inject.injectPostExec(*ipc, pi->hProcess, pi->hThread);
    }

    return ret;
}
#endif


void ExecHookInitialize() {
#ifdef _WIN32
#ifdef USE_DETOURS
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
#endif

    HMODULE kernel32Module = LoadLibrary("kernel32.dll");

    origCreateProcessInternalW = reinterpret_cast<CreateProcessInternalW_Type>(GetProcAddress(kernel32Module, "CreateProcessInternalW"));
        
    if (origCreateProcessInternalW) {
#if defined(USE_DETOURS) || defined(USE_MHOOK)
        void* hookPtr = &CreateProcessInternalW_CALL;
#endif
#ifdef USE_DETOURS
        DetourAttach(&(PVOID&)origAddress, hookPtr);
#endif
#ifdef USE_MHOOK
        if (!Mhook_SetHook(&(PVOID&)origCreateProcessInternalW, hookPtr)) {
            Os::fatal("Cannot hook CreateProcessInternalW function.");
        }
#endif
    }
#ifdef USE_DETOURS
    DetourTransactionCommit();
#endif
#endif
}