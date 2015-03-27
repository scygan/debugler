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

#include "dgladbinterface.h"

class DGLAdbDeviceProcess {
   public:
    DGLAdbDeviceProcess(const std::string& pid, const std::string& name,
                        const std::string& portName = "");

    bool operator==(const DGLAdbDeviceProcess& rhs) const;

    bool operator<(const DGLAdbDeviceProcess& other) const;
    const std::string& getPid() const;
    const std::string& getName() const;
    const std::string& getPortName() const;

    const std::string getDescriptionStr() const;

   private:
    std::string m_Pid;
    std::string m_Name;
    std::string m_PortName;
};

class DGLADBDevice : public QObject, DGLAdbHandler {
    Q_OBJECT
   public:
    DGLADBDevice(const std::string& serial);
    void reloadProcesses();
    void reloadPackages();
    const std::string& getSerial() const;

    void queryStatus();

    void portForward(const std::string& from, unsigned short to);

    void setProcessBreakpoint(const std::string& processName);
    void unsetProcessBreakpoint();

    void installWrapper(const std::string& path);
    void updateWrapper(const std::string& path);
    void uninstallWrapper(const std::string& path);

    enum class InstallStatus {
        UNKNOWN,
        UNAUTHORIZED,
        INSTALLED,
        CLEAN,
    };

    enum class ABI {
        UNKNOWN,
        X86,
        ARMEABI,
        MIPS
    };

    InstallStatus getInstallStatus();
    ABI getABI();

signals:
    void gotProcesses(DGLADBDevice*,
                      const std::vector<DGLAdbDeviceProcess>& data);
    void gotPackages(DGLADBDevice*,
        const std::vector<std::string>& data);
    void failed(DGLADBDevice*, const std::string&);
    void portForwardSuccess(DGLADBDevice*);
    void setProcessBreakPointSuccess(DGLADBDevice*);
    void unsetProcessBreakPointSuccess(DGLADBDevice*);
    void queryStatusSuccess(DGLADBDevice*);
    void installerDone(DGLADBDevice*);
    void log(DGLADBDevice*, const std::string& log);

   private:
    virtual void done(const std::vector<std::string>& data) override;
    virtual void failed(const std::string& reason) override;

    void reloadProcessesGotPortString(const std::vector<std::string>& prop);
    void reloadProcessesGotUnixSockets(const std::vector<std::string>& prop);
    void doneQueryInstallStatus(const std::vector<std::string>& prop);
    void doneQueryABI(const std::vector<std::string>& prop);

    DGLAdbCookie* getProp(const std::string& prop);
    DGLAdbCookie* waitForDevice();
    DGLAdbCookie* checkUser();
    DGLAdbCookie* remountFromAdb();
    DGLAdbCookie* remountFromShell();

    DGLAdbCookie* stopFrameworks();
    DGLAdbCookie* startFramewors();


    DGLAdbCookie* invokeAsShellUser(
            const std::vector<std::string>& params,
            std::shared_ptr<DGLAdbOutputFilter> filter =
                    std::shared_ptr<DGLAdbOutputFilter>());

    DGLAdbCookie* invokeAsRoot(const std::vector<std::string>& params,
                               std::shared_ptr<DGLAdbOutputFilter> filter =
                                       std::shared_ptr<DGLAdbOutputFilter>());

    bool checkIdle();

    std::string m_Serial;
    InstallStatus m_Status;
    ABI m_ABI;

    enum class RequestStatus {
        IDLE,
        RELOAD_PROCESSES,
        RELOAD_PACKAGES,
        SET_BREAKPOINT,
        UNSET_BREAKPOINT,
        QUERY_ABI,
        QUERY_INSTALL_STATUS,
        PREP_INSTALL,
        PREP_UPDATE,
        PREP_UNINSTALL,
        PORT_FORWARD,
    } m_RequestStatus;

    enum class DetailRequestStatus {
        NONE,
        RELOAD_GET_PORTSTR,
        RELOAD_GET_UNIXSOCKETS,
        RELOAD_GET_PACKAGELIST,
        PREP_ADB_WAIT,
        PREP_ADB_CHECKUSER,
        PREP_GET_DEBUGGABLE_FLAG,
        PREP_ADB_ROOT,
        PREP_ADB_CHECK_SU_USER,
        PREP_ADB_CHECK_SU_PARAM_MODE,
        PREP_REMOUNT_FROM_ADB,
        PREP_REMOUNT_FROM_SHELL,
        PREP_FRAMEWORK_STOP,
        PREP_FRAMEWORK_UPLOAD_INSTALLER,
        PREP_FRAMEWORK_SYNC_FLUSH,
        PREP_FRAMEWORK_CHMOD_INSTALLER,
        PREP_FRAMEWORK_RUN_INSTALLER,
        PREP_FRAMEWORK_START,
    } m_DetailRequestStatus;

    const char* toString(RequestStatus status);
    const char* toString(DetailRequestStatus detailStatus);

    void setRequestStatus(RequestStatus status);
    void setRequestStatus(DetailRequestStatus detailStatus);

    QRegExp m_SocketPathRegex;
    int m_PidInSocketRegex;
    int m_PNameInSocketRegex;

    bool m_RootSuRequired;
    bool m_RootSuParamConcat;

    std::string m_InstallerPath;
};

#endif
