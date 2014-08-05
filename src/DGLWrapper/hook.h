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

#include <DGLCommon/def.h>

/**
 * Class implementing detour-based hooking mechanism
 *
 * This class provides API for mHook (or Detours, if compiled in) library
 */
class HookSession {
public:
    /** 
     * Construct a hook session.
     * 
     * Session is required to start code patching.
     * Some hooks are not guaranteed to be active until sessions is destructed.
     */
    HookSession();

    /** 
     * Destruct a hook session.
     * 
     * Commit all pending hooks, guarantee all are active since now.
     */
    ~HookSession();

    /** 
     * Hook a function pointer
     * 
     * @param from original API function to be hooked. The trampoline address is returned here.
     * @param to hook to install into 'from' function code
     * @return true if success
     */
    bool hook(dgl_func_ptr* from, dgl_func_ptr to);
};