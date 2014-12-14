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

#include "exechook.h"
#include "hook.h"
#include "ipc.h"
#include "dl.h"
#include "globalstate.h"

#include <DGLInject/inject.h>
#include <DGLCommon/os.h>

BOOL ExecHook::CreateProcessInternalW(
        HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
        LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
        BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir, LPSTARTUPINFOW si,
        LPPROCESS_INFORMATION pi, PHANDLE NewToken) {

    DGLIPC* ipc = getIPC();

    DGLInject inject(ipc->getWrapperPath());

    BOOL ret = real_CreateProcessInternalW(
            hToken, AppName, CmdLine, ProcessAttr, ThreadAttr, bIH,
            flags | CREATE_SUSPENDED, env, CurrDir, si, pi, NewToken);

    if (ret) {
        inject.injectPostExec(*ipc, pi->hProcess, pi->hThread);
    }

    return ret;
}

BOOL WINAPI CreateProcessInternalW_CALL(
        HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
        LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
        BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir, LPSTARTUPINFOW si,
        LPPROCESS_INFORMATION pi, PHANDLE NewToken) {

    try {
        return ExecHook::CreateProcessInternalW(
                hToken, AppName, CmdLine, ProcessAttr, ThreadAttr, bIH, flags,
                env, CurrDir, si, pi, NewToken);
    } catch (const std::exception& e) {
        Os::fatal(e.what());
    }
}

BOOL ExecHook::real_CreateProcessInternalW(
        HANDLE hToken, LPCWSTR AppName, LPWSTR CmdLine,
        LPSECURITY_ATTRIBUTES ProcessAttr, LPSECURITY_ATTRIBUTES ThreadAttr,
        BOOL bIH, DWORD flags, LPVOID env, LPCWSTR CurrDir, LPSTARTUPINFOW si,
        LPPROCESS_INFORMATION pi, PHANDLE NewToken) {
    return s_real_CreateProcessInternalW(hToken, AppName, CmdLine, ProcessAttr,
                                         ThreadAttr, bIH, flags, env, CurrDir,
                                         si, pi, NewToken);
}

void ExecHook::initialize() {
    std::shared_ptr<DynamicLibrary> kernel32Library = EarlyGlobalState::getDynLoader().getLibrary("kernel32.dll");

    s_real_CreateProcessInternalW =
            reinterpret_cast<CreateProcessInternalW_Type>(kernel32Library->getFunction("CreateProcessInternalW"));

    if (s_real_CreateProcessInternalW) {

        HookSession hookSession;
        dgl_func_ptr hookPtr =
                dgl_func_ptr(&CreateProcessInternalW_CALL);
        if (!hookSession.hook(
                    (dgl_func_ptr*)&s_real_CreateProcessInternalW,
                    hookPtr)) {
            Os::fatal("Cannot hook CreateProcessInternalW function.");
        }
    }
}

ExecHook::CreateProcessInternalW_Type ExecHook::s_real_CreateProcessInternalW;

std::recursive_mutex ExecHook::s_mutex;

#endif