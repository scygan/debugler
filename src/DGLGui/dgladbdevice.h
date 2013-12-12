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

#ifndef DGLADBDEVICE_H
#define DGLADBDEVICE_H

class DGLAdbDeviceProcess {
   public:
    DGLAdbDeviceProcess(const std::string& pid, const std::string& name,
                        const std::string& portName);
    bool operator<(const DGLAdbDeviceProcess& other) const;
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

    void queryInstallStatus();

    void portForward(std::string from, unsigned short to);

    enum class InstallStatus {
        UNKNOWN,
        INSTALLED,
        CLEAN,
    };

    InstallStatus getInstallStatus();

   public
slots:
    void reloadProcessesGotPortString(const std::vector<std::string>& prop);
    void reloadProcessesGotUnixSockets(const std::vector<std::string>& prop);
    void doneQueryInstallStatus(const std::vector<std::string>& prop);
    void adbFailed(std::string reason);
signals:
    void gotProcesses(DGLADBDevice*,
                      const std::vector<DGLAdbDeviceProcess>& data);
    void failed(DGLADBDevice*, const std::string&);

    void portForwardSuccess(DGLADBDevice*);
    void queryInstallStatusSuccess(DGLADBDevice*);

   private:
    std::string m_Serial;
    InstallStatus m_Status;

    QRegExp m_SocketPathRegex;
    int m_PidInSocketRegex;
    int m_PNameInSocketRegex;
};

#endif
