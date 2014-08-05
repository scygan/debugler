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

#include "hook.h"

#include <windows.h>

#ifdef USE_MHOOK
#include "mhook/mhook-lib/mhook.h"
#endif

#ifdef USE_DETOURS
#include "detours/detours.h"
#endif

HookSession::HookSession() {
#ifdef USE_DETOURS
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
#endif
}

HookSession::~HookSession() {
#ifdef USE_DETOURS
    DetourTransactionCommit();
#endif
}

bool HookSession::hook(func_ptr* from, func_ptr to) {
#ifdef USE_DETOURS
    if (DetourAttach(from, to)) {
        return false;
    }
#endif
#ifdef USE_MHOOK
    if (!Mhook_SetHook(reinterpret_cast<PVOID*>(from), to)) {
        return false;
    }
#endif
    return true;
}