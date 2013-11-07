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


#include "dglprocess.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread_time.hpp>
#include <DGLCommon/os.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #define MAX_PATH PATH_MAX
#endif

DGLBaseQTProcess::DGLBaseQTProcess() {}

void DGLBaseQTProcess::run(std::string exec, std::string path, std::vector<std::string> args, bool takeOutput) {
 
    QStringList arguments;
    for (size_t i = 0; i < args.size(); i++) {
        arguments << QString::fromStdString(args[i]);
    }

    if (path.length()) {        
        char absolutePath[MAX_PATH];
#ifdef _WIN32        
        if (!_fullpath(absolutePath, path.c_str(), MAX_PATH)) {
#else
        if (!realpath(path.c_str(), absolutePath)) {
#endif
            throw std::runtime_error(Os::translateOsError(Os::getLastosError()));
        }
        m_process.setWorkingDirectory(absolutePath);
    }

    if (takeOutput) {
        m_process.setProcessChannelMode(QProcess::MergedChannels);
    }

    m_process.start(QString::fromStdString(exec), arguments);
}


void DGLBaseQTProcess::exit(bool wait) {
    m_process.terminate();
    if (wait) {
        m_process.waitForFinished();
    }
}

void DGLBaseQTProcess::requestDelete() {
    //no more signals will be emited
    disconnect();
    
    //if process is still running terminate it
    if (m_process.state() != QProcess::NotRunning) {
        exit(false);
    }

    //request destruction of current object
    deleteLater();
}

DGLDebugeeQTProcess::DGLDebugeeQTProcess(int port, bool modeEGL):
    m_Port(port), m_Loaded(false), m_ModeEGL(modeEGL),
    m_PortStr(boost::lexical_cast<std::string>(getPort())), m_SemLoaderStr("sem_loader_" + m_PortStr),
    m_SemOpenGLStr("sem_" + m_PortStr),
    m_SemLoader(boost::interprocess::open_or_create, m_SemLoaderStr.c_str(), 0),
    m_SemOpenGL(boost::interprocess::open_or_create, m_SemOpenGLStr.c_str(), 0), 
    m_PollTimer(new QTimer(this)){

    CONNASSERT(m_PollTimer, SIGNAL(timeout()), this, SLOT(pollReady()));
    CONNASSERT(&m_process, SIGNAL(started()), this, SLOT(startPolling()));
    CONNASSERT(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleProcessError(QProcess::ProcessError)));
    CONNASSERT(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleProcessFinished(int, QProcess::ExitStatus)));
}

DGLDebugeeQTProcess::~DGLDebugeeQTProcess() {
    m_process.kill();
}

void DGLDebugeeQTProcess::run(std::string cmd, std::string path, std::vector<std::string> args, bool takeOutput) {

    try {
#ifdef _WIN32

        HANDLE file = CreateFile(cmd.c_str(),  GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!file) {
            throw std::runtime_error("Open executable file failed");
        }

        HANDLE hMap = CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr);      
        if (!hMap) {
            CloseHandle(file);
            throw std::runtime_error("Create file mapping failed");
        }

        char* header = MapViewOfFileEx(hMap, FILE_MAP_READ, 0, 0, 0, nullptr);  
        if (!header) {
            CloseHandle(hMap);
            CloseHandle(file);
            throw std::runtime_error("Create file mapping failed");
        }

#include <pshpack1.h>
        const struct IMAGE_HEADER {
            DWORD signature;
            IMAGE_FILE_HEADER fileHeader;
        };
#include <poppack.h>

        char* currentHeader = header;
        unsigned int fileSize = GetFileSize(file, nullptr);

        bool correct = (fileSize > (currentHeader - header) + sizeof(IMAGE_DOS_HEADER));

        if (correct) {
            IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER)currentHeader;
            correct &= dosHeader->e_magic = IMAGE_DOS_SIGNATURE;
            currentHeader += dosHeader->e_lfanew;
            bool correct = (fileSize > (currentHeader - header) + sizeof(IMAGE_HEADER));
        }

        IMAGE_HEADER* iHeader;

        if (correct) {
            iHeader = (IMAGE_HEADER)currentHeader;
            correct &= (iHeader->signature = IMAGE_NT_SIGNATURE);
        }

        if (!correct) {
            throw std::runtime_error("Executable is not a NT PE file.");
        }       

        WORD machine = iHeader->fileHeader.Machine;
        
        CloseHandle(hMap);
        CloseHandle(file);

        std::string loaderPath;
        switch (machine) {
        case IMAGE_FILE_MACHINE_I386:
            loaderPath = "DGLLoader.exe";
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            loaderPath = "DGLLoader64.exe";
            break;
        case IMAGE_FILE_MACHINE_IA64: 
            throw std::runtime_error("Unsupported PE binary format (IA64)");
            break;
        default:
            throw std::runtime_error("Unsupported PE binary format");
        }
#else
        std::string loaderPath = "dglloader";
#endif

        //set environment variables - child processes will inherit these

        //semaphore triggered by loader (when done loading)
        Os::setEnv("dgl_loader_semaphore", m_SemLoaderStr.c_str());

        //semaphore triggered by opengl32.dll (when OpenGL is first used and server is ready)
        Os::setEnv("dgl_semaphore", m_SemOpenGLStr.c_str());


        //shmem for getting loader error
        std::string shmemName = "shmem_" + m_PortStr;
        Os::setEnv("dgl_loader_shmem", shmemName.c_str());
        m_ShObj = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, shmemName.c_str(), boost::interprocess::read_write);
        m_ShObj.truncate(sizeof(IPCMessage));
        m_MappedRegion = boost::interprocess::mapped_region(m_ShObj, boost::interprocess::read_write);

        //run loader process

        std::vector<std::string> arguments;

        if (m_ModeEGL) {
            arguments.push_back("--egl");
        }
        arguments.push_back("--port");
        arguments.push_back("tcp:" + m_PortStr);

        arguments.push_back(cmd);

        if (args.size()) {
            arguments.push_back("--");
        }

        for (size_t i = 0; i < args.size(); i++) {
            arguments.push_back(args[i]);
        }

        DGLBaseQTProcess::run(loaderPath, path, arguments, takeOutput);

    } catch (const std::runtime_error& err) {
        if (int osError = Os::getLastosError()) {
            std::ostringstream message;
            message <<  err.what() << ": " << Os::translateOsError(osError);
            throw std::runtime_error(message.str());
        } else {
            throw err;
        }
    }
}

void DGLDebugeeQTProcess::startPolling() {
    m_PollTimer->start(10);
}

void DGLDebugeeQTProcess::pollReady() {
    if (m_Loaded) {
        if (m_SemOpenGL.try_wait()) {
            m_PollTimer->stop();
            emit processReady();
        }
    } else {
        if (m_SemLoader.try_wait()) {
            m_Loaded = true;
            pollReady();
        }
    }
}

void DGLDebugeeQTProcess::handleProcessError(QProcess::ProcessError) {
    emit processError(m_process.errorString().toStdString());
}

void DGLDebugeeQTProcess::handleProcessFinished(int code, QProcess::ExitStatus status) {
    
    m_PollTimer->stop();

    if (status == QProcess::CrashExit) {
        //this is loader process that crashed!
        emit processCrashed();
    } else {
        //check in IPC if loader was successful
        if (m_Loaded ||  m_SemLoader.try_wait()) {
            IPCMessage* ipcMessage = (IPCMessage*)m_MappedRegion.get_address();

            if (ipcMessage) {
                static_assert(sizeof(char) == sizeof(ipcMessage->m_message[0]), "Wrong IPC message element size");
                if (!ipcMessage->m_ok) {
                    emit processError((char*)ipcMessage->m_message);
                }
            }
        }
        emit processFinished(code);
    }
}

int DGLDebugeeQTProcess::getPort() {
    return m_Port;
}

