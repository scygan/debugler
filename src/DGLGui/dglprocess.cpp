#include "dglprocess.h"

#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <DGLCommon/os.h>

#include <windows.h>

#ifdef _WIN32

class DGLProcessImpl: public DGLProcess {

    struct IPCMessage {
        IPCMessage(uint32_t s):status(s) { message[0] = 0; };
        uint32_t status;
        char message[1000];
    };

public:

    DGLProcessImpl(std::string exec, std::string path, std::string args, int port):
        m_PortStr(boost::lexical_cast<std::string>(port)), m_SemLoaderStr("sem_loader_" + m_PortStr),
        m_SemOpenGLStr("sem_" + m_PortStr),
        m_SemLoader(boost::interprocess::create_only, m_SemLoaderStr.c_str(), 0),
        m_SemOpenGL(boost::interprocess::create_only, m_SemOpenGLStr.c_str(), 0)
    {

        DWORD binaryType;
        if (!GetBinaryTypeA(exec.c_str(), &binaryType)) {
            throw std::runtime_error("Getting binary file type failed");
        }

        std::string loaderPath;
        switch (binaryType) {
            case SCS_32BIT_BINARY:
                loaderPath = "DGLLoader.exe";
                break;
            case SCS_64BIT_BINARY:
                loaderPath = "DGLLoader64.exe";
                break;
            default:
                throw std::runtime_error("Unsupported PE binary format");
        }

        std::stringstream portStr;  portStr << port;

        //set environment variables - child processes will inherit these

        //debugging port
        SetEnvironmentVariableA("dgl_port", portStr.str().c_str());

        //semaphore triggered by loader (when done loading)
        SetEnvironmentVariableA("dgl_loader_semaphore", m_SemLoaderStr.c_str());

        //semaphore triggered by opengl32.dll (when OpenGL is first used and server is ready)
        SetEnvironmentVariableA("dgl_semaphore", m_SemOpenGLStr.c_str());
        

        //shmem for getting loader error
        std::string shmemName = "shmem_" + portStr.str();
        SetEnvironmentVariableA("dgl_loader_shmem", shmemName.c_str());
        m_ShObj = boost::interprocess::shared_memory_object(boost::interprocess::create_only, shmemName.c_str(), boost::interprocess::read_write);
        m_ShObj.truncate(sizeof(IPCMessage));
        m_MappedRegion = boost::interprocess::mapped_region(m_ShObj, boost::interprocess::read_write);
       

        //prepare some structures for CreateProcess output
        STARTUPINFOA startupInfo;
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        PROCESS_INFORMATION processInformation; 
        memset(&processInformation, 0, sizeof(processInformation));

        //run loader process

        std::string arguments = 
            "\"" + loaderPath + "\" " +
            "\"" + exec + "\" " +
            "\"" + args + "\" ";

        if (CreateProcessA(
            (LPSTR)loaderPath.c_str(),
            (LPSTR)arguments.c_str(),
            NULL, 
            NULL,
            FALSE, 
            0,
            NULL,
            path.c_str(),
            &startupInfo, 
            &processInformation) == 0 ) {

                throw std::runtime_error("Cannot create loader process");
        }
    };


    bool waitOpenGL(int msec) {
        return m_SemOpenGL.timed_wait(boost::get_system_time() + boost::posix_time::milliseconds(msec));
    }

    bool waitLoader(int msec) {
        if (m_SemLoader.timed_wait(boost::get_system_time() + boost::posix_time::milliseconds(msec))) {
             
            IPCMessage* ipcMessage = (IPCMessage*)m_MappedRegion.get_address();

            if (ipcMessage->status != EXIT_SUCCESS) {
                throw std::runtime_error(ipcMessage->message);
            }

            return true;
        } else {
            return false; //timeout
        }
    }


    virtual bool waitReady(int msec) {
        if (m_Loaded) {
            return waitOpenGL(msec);
        } else {
            return waitLoader(msec);
        }
    }


    bool m_Loaded;

    std::string m_PortStr, m_SemLoaderStr, m_SemOpenGLStr;

    boost::interprocess::named_semaphore m_SemLoader, m_SemOpenGL;
    boost::interprocess::shared_memory_object  m_ShObj;
    boost::interprocess::mapped_region m_MappedRegion;
};

#else

class DGLProcessImpl: public DGLProcess {
    DGLProcessImpl(std::string cmd, std::string path, std::string args, int port) {}

    virtual bool waitReady(int msec) {
        return true;
    }
};

#endif



DGLProcess* DGLProcess::Create(std::string exec, std::string path, std::string args, int port) {
    return new DGLProcessImpl(exec, path, args, port);
}