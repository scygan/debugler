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

#include <DGLGui/dglprocess.h>
#include <DGLGui/dglgui.h>

#include <QEventLoop>

class LiveProcessWrapper : public QObject {
    Q_OBJECT
   public:
    LiveProcessWrapper(std::string sampleName) : m_Done(false) {

        m_process = new DGLDebugeeQTProcess(8888, false);

        m_process->setParent(this);

        CONNASSERT(m_process, SIGNAL(processReady()), this,
                   SLOT(processReadyHandler()));
        CONNASSERT(m_process, SIGNAL(processError(std::string)), this,
                   SLOT(processErrorHandler(std::string)));
        CONNASSERT(m_process, SIGNAL(processCrashed()), this,
                   SLOT(processCrashHandler()));

        std::vector<std::string> args;
#ifdef _WIN32
        std::string exec = "samples\\samples.exe";
        args.push_back(sampleName);
#else
        std::string exec = "../samples/samples";
        args.push_back(sampleName);
#endif
        QEventLoop loop;
        CONNASSERT(this, SIGNAL(done()), &loop, SLOT(quit()));

        m_process->run(exec, "", args);

        if (!m_Done) loop.exec();

        if (m_errorInfo.size()) {
            throw std::runtime_error(m_errorInfo);
        }
    }

    ~LiveProcessWrapper() {
        disconnect();
        m_process->exit(true);
        delete m_process;
    }

    void waitForSocket() {
        m_process->waitForSocket();
    }

signals:
    void done();

   private
slots:
    void processErrorHandler(std::string errorMsg) {
        m_errorInfo = errorMsg;
        emit done();
        m_Done = true;
    }
    void processCrashHandler() {
        m_errorInfo = "Process crashed";
        emit done();
        m_Done = true;
    }

    void processReadyHandler() {
        emit done();
        m_Done = true;
    }

   private:
    DGLDebugeeQTProcess* m_process;
    std::string m_errorInfo;
    bool m_Done;
};
