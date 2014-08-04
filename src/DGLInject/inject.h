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
#endif

#include <string>

#include <DGLCommon/ipc.h>

class DGLInject {
public:

    DGLInject(const std::string& wrapperPath);

#ifdef _WIN32
    typedef HANDLE native_process_handle_t;
#else
    typedef pid_t native_process_handle_t;
#endif

    void injectPostFork(); 

    void injectPostExec(DGLIPC& ipc, native_process_handle_t processHandle, native_process_handle_t mainThreadHandle); 

private:
    std::string m_WrapperPath;
};