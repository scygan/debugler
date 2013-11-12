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

#ifndef DGLADBINTERFACE_H
#define DGLADBINTERFACE_H

#include <memory>
#include <string>
#include <vector>

#include <QRegExp>

#include "dglprocess.h"

class DGLAdbOutputFilter;

class DGLAdbCookie : public DGLBaseQTProcess {
    Q_OBJECT
   public:
    DGLAdbCookie(const std::string& adbPath,
                 const std::vector<std::string>& params,
                 std::shared_ptr<DGLAdbOutputFilter> filter);

    void process();

signals:
    void done(std::vector<std::string> data);
    void failed(std::string reason);

   private
slots:
    void handleProcessError(QProcess::ProcessError);
    void handleProcessFinished(int, QProcess::ExitStatus);

   private:
    std::string m_adbPath;
    std::vector<std::string> m_params;
    std::shared_ptr<DGLAdbOutputFilter> m_OutputFilter;
};

class DGLAdbProcess {
   public:
    DGLAdbProcess(const std::string& pid, const std::string& name,
                  const std::string& portName);
    bool operator<(const DGLAdbProcess& other);
    const std::string& getPid() const;
    const std::string& getName() const;
    const std::string& getPortName() const;

   private:
    std::string m_Pid;
    std::string m_Name;
    std::string m_PortName;
};

class DGLADBDevice : public QObject {
    Q_OBJECT
   public:
    DGLADBDevice(const std::string& serial);
    void reloadProcesses();
    const std::string& getSerial() const;
   public
slots:
    void reloadProcessesGotPortString(const std::vector<std::string>& prop);
    void reloadProcessesGotUnixSockets(const std::vector<std::string>& prop);
    void adbFailed(std::string reason);
signals:
    void gotProcesses(const std::vector<DGLAdbProcess>& data);
    void failed(DGLADBDevice*, const std::string&);

   private:
    std::string m_Serial;

    QRegExp m_SocketPathRegex;
    int m_PidInSocketRegex;
    int m_PNameInSocketRegex;
};

class DGLAdbInterface {
   public:
    static DGLAdbInterface* get();

    void setAdbPath(const std::string& path);
    const std::string& getAdbPath() const;

    DGLAdbCookie* killServer();
    DGLAdbCookie* connect(const std::string& address);
    DGLAdbCookie* getDevices();

    DGLAdbCookie* invokeOnDevice(const std::string& serial,
                                 const std::vector<std::string>& params,
                                 std::shared_ptr<DGLAdbOutputFilter> filter =
                                         std::shared_ptr<DGLAdbOutputFilter>());

   private:
    DGLAdbCookie* invokeAdb(const std::vector<std::string>& params,
                            std::shared_ptr<DGLAdbOutputFilter> filter =
                                    std::shared_ptr<DGLAdbOutputFilter>());

    std::string m_adbPath;

    static std::shared_ptr<DGLAdbInterface> s_self;
};

#endif
