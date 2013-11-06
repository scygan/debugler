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
#pragma warning(disable:4996) 
#include "ipc.h" //intentionally inside warning disable, as next header instances some warning-full lcode here.
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
#include <boost/make_shared.hpp>
#include <sstream>

#include <DGLCommon/def.h>

#pragma warning(disable:4503)

class DGLIPCImpl: public DGLIPC {
public:

    DGLIPCImpl():m_region(NULL), m_regionowner(true) {
        std::ostringstream uuidStream;
        uuidStream << boost::uuids::random_generator()();
        m_uuid = uuidStream.str();
#ifdef _WIN32
        m_shmem = boost::make_shared<boost::interprocess::windows_shared_memory>(boost::interprocess::create_only, m_uuid.c_str(), boost::interprocess::read_write, sizeof(MemoryRegion));
#else
        m_shmem = boost::make_shared<boost::interprocess::shared_memory_object>(boost::interprocess::create_only, m_uuid.c_str(), boost::interprocess::read_write);
        m_shmem->truncate(sizeof(MemoryRegion));
#endif
        m_shmemregion = boost::make_shared<boost::interprocess::mapped_region>(*m_shmem, boost::interprocess::read_write);

        //inplace
        m_region = new(m_shmemregion->get_address()) MemoryRegion;
    }

    DGLIPCImpl(std::string uuid):m_uuid(uuid),m_region(NULL), m_regionowner(false) {
#ifdef _WIN32
        m_shmem = boost::make_shared<boost::interprocess::windows_shared_memory>(boost::interprocess::open_only, m_uuid.c_str(), boost::interprocess::read_write);
#else
        m_shmem = boost::make_shared<boost::interprocess::shared_memory_object>(boost::interprocess::open_only, m_uuid.c_str(), boost::interprocess::read_write);
#endif
        m_shmemregion = boost::make_shared<boost::interprocess::mapped_region>(*m_shmem, boost::interprocess::read_write);
        m_region = reinterpret_cast<MemoryRegion*>(m_shmemregion->get_address());
    }

    ~DGLIPCImpl() {
        if (m_regionowner && m_region) {
            m_region->~MemoryRegion();
        }
    }

    virtual const std::string& getUUID() override {
        return m_uuid;
    }
    virtual void waitForRemoteThreadSemaphore() override {
        m_region->m_remoteThreadSemaphore.wait();
    }
    virtual void postRemoteThreadSemaphore() override {
        m_region->m_remoteThreadSemaphore.post();
    }

    virtual void setWaitForConnection(bool wait) override {
        m_region->m_WaitForConnection = wait;
    }

    virtual bool getWaitForConnection() override {
        return m_region->m_WaitForConnection;
    }

    virtual void setDebuggerMode(DebuggerMode mode) override {
        m_region->m_debuggerMode = mode;
    }

    virtual DebuggerMode getDebuggerMode() override {
        return m_region->m_debuggerMode;
    }

    virtual void setDebuggerPort(DebuggerPortType type, const std::string& port) override {
        m_region->m_debuggerPortType = type;
        strncpy(m_region->m_debuggerPortName, port.c_str(), c_debuggerPortNameLen);
        m_region->m_debuggerPortName[c_debuggerPortNameLen - 1] = '\0';
    }

    virtual DebuggerPortType getDebuggerPort(std::string& port) override {
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

private:

    static const int c_debuggerPortNameLen = 1000;

    struct MemoryRegion {
        MemoryRegion():m_debuggerPortType(DebuggerPortType::TCP),m_debuggerMode(DebuggerMode::DEFAULT),
            m_remoteThreadSemaphore(0), m_WaitForConnection(true), m_DLInterceptDlOpenAddr(0), m_DLInterceptDlSymAddr(0) {
            strncpy(m_debuggerPortName, "5555", c_debuggerPortNameLen);
        }
        char m_debuggerPortName[c_debuggerPortNameLen];
        DebuggerPortType m_debuggerPortType;
        DebuggerMode m_debuggerMode;
        boost::interprocess::interprocess_semaphore m_remoteThreadSemaphore;
        bool m_WaitForConnection;
        int m_DLInterceptDlOpenAddr;
        int m_DLInterceptDlSymAddr;
    };

    std::string m_uuid;

    MemoryRegion* m_region;

#ifdef _WIN32
    boost::shared_ptr<boost::interprocess::windows_shared_memory> m_shmem;
#else
    boost::shared_ptr<boost::interprocess::shared_memory_object> m_shmem;
#endif
    boost::shared_ptr<boost::interprocess::mapped_region> m_shmemregion;
    bool m_regionowner;
};




std::shared_ptr<DGLIPC> DGLIPC::Create() {
    return std::shared_ptr<DGLIPC>(new DGLIPCImpl());
}

std::shared_ptr<DGLIPC> DGLIPC::CreateFromUUID(std::string uuid) {
    return std::shared_ptr<DGLIPC>(new DGLIPCImpl(uuid));
}








