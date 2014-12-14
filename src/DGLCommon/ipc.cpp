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

#pragma warning(push)
#pragma warning(disable : 4996)
#include "ipc.h"    //intentionally inside warning disable, as next header instances some warning-full lcode here.
#include <boost/uuid/uuid.hpp>
#pragma warning(pop)

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#ifdef _WIN32
#include <boost/interprocess/windows_shared_memory.hpp>
#else
#include <boost/interprocess/shared_memory_object.hpp>
#endif
#include <boost/interprocess/mapped_region.hpp>
#include <sstream>

#include <DGLCommon/def.h>

#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

#pragma warning(disable : 4503)

class DGLIPCImpl : public DGLIPC {
   public:
    DGLIPCImpl(const std::string& wrapperPath, DebuggerMode debuggerMode, DebuggerListenMode listenMode, DebuggerPortType portType, const char* portName, int processSkipCount) : m_region(NULL), m_processSkipped(false), m_regionowner(true) {
        
        DGL_ASSERT(listenMode != DebuggerListenMode::NO_LISTEN);

        DGL_ASSERT(wrapperPath.size());
        
        std::ostringstream uuidStream;
        uuidStream << boost::uuids::random_generator()();
        m_uuid = uuidStream.str();
#ifdef _WIN32
        m_shmem =
                std::make_shared<boost::interprocess::windows_shared_memory>(
                        boost::interprocess::create_only, m_uuid.c_str(),
                        boost::interprocess::read_write, sizeof(MemoryRegion));
#else
        m_shmem = std::make_shared<boost::interprocess::shared_memory_object>(
                boost::interprocess::create_only, m_uuid.c_str(),
                boost::interprocess::read_write);
        m_shmem->truncate(sizeof(MemoryRegion));
#endif
        m_shmemregion = std::make_shared<boost::interprocess::mapped_region>(
                *m_shmem, boost::interprocess::read_write);

        // inplace
        m_region = new (m_shmemregion->get_address()) MemoryRegion(wrapperPath, debuggerMode, listenMode, portType, portName, processSkipCount);
    }

    DGLIPCImpl(std::string uuid)
            : m_uuid(uuid), m_region(NULL), m_processSkipped(false), m_regionowner(false) {
#ifdef _WIN32
        m_shmem =
                std::make_shared<boost::interprocess::windows_shared_memory>(
                        boost::interprocess::open_only, m_uuid.c_str(),
                        boost::interprocess::read_write);
#else
        m_shmem = std::make_shared<boost::interprocess::shared_memory_object>(
                boost::interprocess::open_only, m_uuid.c_str(),
                boost::interprocess::read_write);
#endif
        m_shmemregion = std::make_shared<boost::interprocess::mapped_region>(
                *m_shmem, boost::interprocess::read_write);
        m_region =
                reinterpret_cast<MemoryRegion*>(m_shmemregion->get_address());
    }

    ~DGLIPCImpl() {
        if (m_regionowner && m_region) {
            m_region->~MemoryRegion();
        }
    }

    virtual const std::string& getUUID() override { return m_uuid; }
    virtual void waitForRemoteThreadSemaphore() override {
        m_region->m_remoteThreadSemaphore.wait();
    }
    virtual void postRemoteThreadSemaphore() override {
        m_region->m_remoteThreadSemaphore.post();
    }

    virtual void newProcessNotify() override {
        m_processSkipped = (std::atomic_fetch_add(&m_region->m_processSkipCounter, -1) >= 0);
    }

    virtual DebuggerListenMode getCurrentProcessListenMode() override {

        if (m_processSkipped) {
            //not listening in when current processes should be skipped
            return DebuggerListenMode::NO_LISTEN;
        }

        return m_region->m_debuggerListenMode;
    }

    virtual DebuggerMode getDebuggerMode() override {
        return m_region->m_debuggerMode;
    }

    virtual DebuggerPortType getDebuggerPort(std::string& port) override {
        
        DGL_ASSERT(getCurrentProcessListenMode() != DebuggerListenMode::NO_LISTEN);

        m_region->m_debuggerPortName[c_debuggerPortNameLen - 1] = '\0';
        port = m_region->m_debuggerPortName;
        return m_region->m_debuggerPortType;
    }

    void getDLInternceptPointers(int& dlOpenAddr, int& dlSymAddr) override {
        dlOpenAddr = m_region->m_DLInterceptDlOpenAddr;
        dlSymAddr = m_region->m_DLInterceptDlSymAddr;
    }

    void setDLInternceptPointers(int dlOpenAddr, int dlSymAddr) override {
        m_region->m_DLInterceptDlOpenAddr = dlOpenAddr;
        m_region->m_DLInterceptDlSymAddr = dlSymAddr;
    }

    virtual const char* getWrapperPath() override {
        return m_region->m_wrapperPath;
    }

   private:
    static const int c_debuggerPortNameLen = 1000;

    struct MemoryRegion {
        MemoryRegion(const std::string& wrapperPath, DebuggerMode debuggerMode, DebuggerListenMode listenMode, DebuggerPortType portType, const char* portName, int processSkipCount)
                : m_debuggerPortType(portType),
                  m_debuggerMode(debuggerMode),
                  m_remoteThreadSemaphore(0),
                  m_debuggerListenMode(listenMode),
                  m_DLInterceptDlOpenAddr(0),
                  m_DLInterceptDlSymAddr(0)
        {
            m_processSkipCounter = processSkipCount;
            dgl_strncpy(m_debuggerPortName, portName, c_debuggerPortNameLen);
            m_debuggerPortName[c_debuggerPortNameLen - 1] = '\0';
            dgl_strncpy(m_wrapperPath, wrapperPath.c_str(), DGL_ARRAY_LENGTH(m_wrapperPath));
        }
        char m_debuggerPortName[c_debuggerPortNameLen];
        DebuggerPortType m_debuggerPortType;
        DebuggerMode m_debuggerMode;
        boost::interprocess::interprocess_semaphore m_remoteThreadSemaphore;
        DebuggerListenMode m_debuggerListenMode;
        int m_DLInterceptDlOpenAddr;
        int m_DLInterceptDlSymAddr;
        std::atomic_int m_processSkipCounter;
        char m_wrapperPath[DGL_MAX_PATH];
    };

    std::string m_uuid;

    MemoryRegion* m_region;

    bool m_processSkipped;

#ifdef _WIN32
    std::shared_ptr<boost::interprocess::windows_shared_memory> m_shmem;
#else
    std::shared_ptr<boost::interprocess::shared_memory_object> m_shmem;
#endif
    std::shared_ptr<boost::interprocess::mapped_region> m_shmemregion;
    bool m_regionowner;
};

std::shared_ptr<DGLIPC> DGLIPC::Create(const std::string& wrapperPath, DebuggerMode debuggerMode, DebuggerListenMode listenMode, DebuggerPortType portType, const char* portName, int processSkipCount) {
    return std::shared_ptr<DGLIPC>(new DGLIPCImpl(wrapperPath, debuggerMode, listenMode, portType, portName, processSkipCount));
}

std::shared_ptr<DGLIPC> DGLIPC::CreateFromUUID(const std::string& uuid) {
    return std::shared_ptr<DGLIPC>(new DGLIPCImpl(uuid));
}
