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


#ifndef DGLPROCESS_H
#define DGLPROCESS_H


#include<string>
#include<memory>

#include <QProcess>
#include <QTimer>

#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

class DGLBaseQTProcess: public QObject {

    Q_OBJECT
public:
    DGLBaseQTProcess();

    virtual void run(std::string cmd, std::string path, std::vector<std::string> args, bool takeOutput = false);
    void exit();

    signals:
        void processEvent(bool ok, std::string errormsg);

    private slots:
        void processStarted();
        void processError(QProcess::ProcessError);
protected:
    QProcess m_process;
};


class DGLDebugeeQTProcess: public DGLBaseQTProcess {
    Q_OBJECT
public:
    DGLDebugeeQTProcess(int port, bool modeEGL);
    virtual ~DGLDebugeeQTProcess();
    
    int getPort();

    virtual void run(std::string cmd, std::string path, std::vector<std::string> args, bool takeOutput = false);

    static DGLDebugeeQTProcess* Create();

signals:
    void processReady();
    
private slots:
    void startPolling();
    void pollReady();

private:

    struct IPCMessage {
        IPCMessage(uint32_t s):status(s) { message[0] = 0; };
        uint32_t status;
        char message[1000];
    };


    int m_Port;
    bool m_Loaded;
    bool m_ModeEGL;

    std::string m_PortStr, m_SemLoaderStr, m_SemOpenGLStr;

    boost::interprocess::named_semaphore m_SemLoader, m_SemOpenGL;
    boost::interprocess::shared_memory_object  m_ShObj;
    boost::interprocess::mapped_region m_MappedRegion;

    QTimer* m_PollTimer;
};

#endif //DGLPROGRAMVIEW_H
