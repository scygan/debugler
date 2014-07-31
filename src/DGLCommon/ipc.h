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

#ifndef IPC_H
#define IPC_H

#include <string>
#include <memory>

class DGLIPC {
   public:
    virtual ~DGLIPC() {}
    virtual const std::string& getUUID() = 0;
    virtual void waitForRemoteThreadSemaphore() = 0;
    virtual void postRemoteThreadSemaphore() = 0;

    enum class DebuggerMode {
        DEFAULT,
        EGL
    };

    enum class DebuggerPortType {
        TCP,
        UNIX
    };

    enum class DebuggerListenMode {
        LISTEN_AND_WAIT,
        LISTEN_NO_WAIT,
        NO_LISTEN,
    };

    virtual void setListenMode(DebuggerListenMode) = 0;
    virtual DebuggerListenMode getCurrentProcessListenMode() = 0;

    virtual void setDebuggerMode(DebuggerMode) = 0;
    virtual DebuggerMode getDebuggerMode() = 0;

    virtual void setDebuggerPort(DebuggerPortType, const std::string&) = 0;
    virtual DebuggerPortType getDebuggerPort(std::string&) = 0;

    virtual void getDLInternceptPointers(int& dlOpenAddr, int& dlSymAddr) = 0;
    virtual void setDLInternceptPointers(int dlOpenAddr, int dlSymAddr) = 0;

    virtual void setNumberOfSkippedProcesses(int processes) = 0;

    static std::shared_ptr<DGLIPC> Create();
    static std::shared_ptr<DGLIPC> CreateFromUUID(std::string uuid);

   protected:
    DGLIPC() {}
};

#endif
