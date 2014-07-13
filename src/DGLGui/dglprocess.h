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

#include <string>
#include <memory>

#include <QProcess>
#include <QTimer>

#ifndef Q_MOC_RUN
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#endif

class DGLBaseQTProcess : public QObject {

    Q_OBJECT
   public:
    DGLBaseQTProcess();

    /**
     * Starts process with parameters
     */
    virtual void run(std::string cmd, std::string path,
                     std::vector<std::string> args, bool takeOutput = false);

    /**
     * Request process exit
     *
     * Signals may be emitted from derived classes.
     */
    void exit(bool wait);

    /**
     * Request process exit and deletion of current object
     *
     * No signals will be emited
     */
    void requestDelete();


    /** 
     * Accessor to underlying  process 
     */
    inline QProcess* getProcess()  { return &m_process; }

   private:
    QProcess m_process;
};

class DGLDebugeeQTProcess : public DGLBaseQTProcess {
    Q_OBJECT
   public:
    DGLDebugeeQTProcess(int port, bool modeEGL);
    virtual ~DGLDebugeeQTProcess();

    int getPort();

    virtual void run(std::string cmd, std::string path,
                     std::vector<std::string> args, bool takeOutput = false);

    static DGLDebugeeQTProcess* Create();

    bool waitForSocket(bool nowait = false);

signals:
    void processReady();
    void processError(std::string errorMsg);
    void processFinished(int);
    void processCrashed();

   private
slots:
    void startPolling();
    void pollReady();

    void handleProcessError(QProcess::ProcessError);
    void handleProcessFinished(int, QProcess::ExitStatus);

   private:
    struct IPCMessage {
        IPCMessage() : m_ok(true) {
            m_message[0] = 0;
        };
        int8_t m_message[1000];
        int8_t m_ok;
    };

    int m_Port;
    bool m_Loaded;
    bool m_ModeEGL;

    std::string m_PortStr, m_SemLoaderStr, m_SemOpenGLStr;

    boost::interprocess::named_semaphore m_SemLoader, m_SemOpenGL;
    boost::interprocess::shared_memory_object m_ShObj;
    boost::interprocess::mapped_region m_MappedRegion;

    QTimer* m_PollTimer;
};

#endif    // DGLPROGRAMVIEW_H
