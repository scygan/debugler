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
#endif

#include "dglgui.h"


DGLBaseQTProcess::DGLBaseQTProcess() {
 

    CONNASSERT(connect(&m_process, SIGNAL(started()), this, SLOT(processStarted())));
    CONNASSERT(connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError))));
    
};

void DGLBaseQTProcess::run(std::string exec, std::string path, std::vector<std::string> args, bool takeOutput) {
 
    QStringList arguments;
    for (size_t i = 0; i < args.size(); i++) {
        arguments << QString::fromStdString(args[i]);
    }

    if (path.length()) {
        char absolutePath[MAX_PATH];
        _fullpath(absolutePath, path.c_str(), MAX_PATH);
        m_process.setWorkingDirectory(absolutePath);
    }

    m_process.start(QString::fromStdString(exec), arguments);
}


void DGLBaseQTProcess::exit() {
    disconnect();
    m_process.terminate();
    deleteLater();
}

void DGLBaseQTProcess::processStarted() {
    emit processEvent(true, "");
}

void DGLBaseQTProcess::processError(QProcess::ProcessError) {
    emit processEvent(false, m_process.errorString().toStdString());
}


DGLDebugeeQTProcess::DGLDebugeeQTProcess(int port, bool modeEGL):
    m_Port(port), m_Loaded(false), m_ModeEGL(modeEGL),
    m_PortStr(boost::lexical_cast<std::string>(getPort())), m_SemLoaderStr("sem_loader_" + m_PortStr),
    m_SemOpenGLStr("sem_" + m_PortStr),
    m_SemLoader(boost::interprocess::open_or_create, m_SemLoaderStr.c_str(), 0),
    m_SemOpenGL(boost::interprocess::open_or_create, m_SemOpenGLStr.c_str(), 0), 
    m_PollTimer(new QTimer(this)){

    CONNASSERT(connect(m_PollTimer, SIGNAL(timeout()), this, SLOT(pollReady())));
    CONNASSERT(connect(&m_process, SIGNAL(started()), this, SLOT(startPolling())));
}

DGLDebugeeQTProcess::~DGLDebugeeQTProcess() {
    m_process.kill();
}

void DGLDebugeeQTProcess::run(std::string cmd, std::string path, std::vector<std::string> args, bool takeOutput) {

    try {
#ifdef _WIN32
        DWORD binaryType;
        if (!GetBinaryTypeA(cmd.c_str(), &binaryType)) {
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
        arguments.push_back(m_PortStr);

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
        if (m_SemOpenGL.timed_wait(boost::get_system_time()/* + boost::posix_time::milliseconds(1)*/)) {
            m_PollTimer->stop();
            emit processReady();
        }
    } else {
        if (m_SemLoader.timed_wait(boost::get_system_time()/* + boost::posix_time::milliseconds(1)*/)) {

            IPCMessage* ipcMessage = (IPCMessage*)m_MappedRegion.get_address();

            if (ipcMessage->status != EXIT_SUCCESS) {
                m_PollTimer->stop();
                emit processEvent(false, ipcMessage->message);
            }

            m_Loaded = true;
            pollReady();
        }
    }
}

int DGLDebugeeQTProcess::getPort() {
    return m_Port;
}

