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


#ifndef PROCESS_H
#define PROCESS_H


#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>
#include <string>

class DGLProcess {
public:
    DGLProcess(std::string executable, std::vector<std::string> args, bool forceFork = false);
    

#ifdef _WIN32
    typedef HANDLE native_process_handle_t ;
#else
    typedef pid_t native_process_handle_t ;
#endif
    native_process_handle_t getHandle();


#ifndef _WIN32
    void do_execvp();
#endif

#ifdef _WIN32
    native_process_handle_t getMainThread();
#endif



private:
#ifdef _WIN32
    PROCESS_INFORMATION m_processInfo;
#else
    native_process_handle_t m_pid;
#endif
    std::string m_executable;
    std::vector<std::string> m_args;



};


#endif

