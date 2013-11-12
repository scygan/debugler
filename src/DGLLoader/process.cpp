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

#include "process.h"

#include <stdexcept>

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

DGLProcess::DGLProcess(std::string executable, std::vector<std::string> args,
                       bool forceFork)
        : m_executable(executable), m_args(args) {
#ifdef _WIN32
    // prepare some structures for CreateProcess output
    STARTUPINFOA startupInfo;
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    memset(&m_processInfo, 0, sizeof(m_processInfo));

    std::string argumentString;
    for (size_t i = 0; i < m_args.size(); i++) {
        argumentString += (i > 0 ? " " : "") + m_args[i];
    }

    char path[MAX_PATH];
    if (!GetCurrentDirectory(MAX_PATH, path)) {
        throw std::runtime_error("GetCurrentDirectory failed");
    }

    // try run process (suspended - will not run user thread)
    if (CreateProcessA(executable.c_str(), (LPSTR)argumentString.c_str(), NULL,
                       NULL, FALSE, CREATE_SUSPENDED, NULL, path, &startupInfo,
                       &m_processInfo) == 0) {

        throw std::runtime_error("Cannot create process");
    }

#else

    m_pid = 0;

#ifdef __ANDROID__
    if (forceFork) {
// on Android fork only, if force fork is set.
// app_process does not like to be forked.
#else
    if (true) {
// on other systems always forks.
#endif
        m_pid = fork();
        if (m_pid == -1) {
            throw std::runtime_error("Cannot fork process");
        }
    }
#endif
}

DGLProcess::native_process_handle_t DGLProcess::getHandle() {
#ifdef _WIN32
    return m_processInfo.hProcess;
#else
    return m_pid;
#endif
}

#ifdef _WIN32
DGLProcess::native_process_handle_t DGLProcess::getMainThread() {
    return m_processInfo.hThread;
}
#endif

#ifndef _WIN32
void DGLProcess::do_execvp() {

    std::vector<std::vector<char> > argvs(m_args.size());
    std::vector<char*> argVector(m_args.size());
    for (size_t i = 0; i < m_args.size(); i++) {
        std::copy(m_args[i].begin(), m_args[i].end(),
                  std::back_inserter<std::vector<char> >(argvs[i]));
        argvs[i].push_back('\0');
        argVector[i] = &argvs[i][0];
    }
    argVector.push_back(NULL);

    execvp(m_executable.c_str(), &argVector[0]);

    throw std::runtime_error("Cannot execute process");
}
#endif
