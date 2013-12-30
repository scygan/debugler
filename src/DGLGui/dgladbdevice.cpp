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

#include "dgladbdevice.h"

namespace {

class DGLEmptyOutputFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>&) override {

        for (size_t i = 0; i < input.size(); i++) {
            if (input[i].size()) {
                return false;
            }
        }
        return true;
    }
};

class DGLGetPropOutputFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) override {
        if (input.size() > 2 || input.size() < 1 ||
            input[0].find("not found") != std::string::npos) {
            return false;
        }
        output.push_back(input[0]);
        return true;
    }
};

class DGLUnixSocketsFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) override {
        if (!input.size()) return false;
        size_t pathOffset = input[0].find("Path");

        if (pathOffset == std::string::npos) {
            return false;
        }

        for (size_t i = 1; i < input.size(); i++) {
            if (input[i].size() > pathOffset) {
                output.push_back(input[i].substr(pathOffset));
            }
        }
        output.push_back(input[0]);
        return true;
    }
};

class DGLTransferFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) override {
        if (input.size() > 2 || input.size() < 1 ||
            input[0].find("bytes in") == std::string::npos) {
            return false;
        }
        output.push_back(input[0]);
        return true;
    }
};

class DGLInstallerFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) override {
        output = input;
        if (input.size() < 2 ||
            input[input.size() - 2].find("Success") == std::string::npos) {
            return false;
        }
        return true;
    }
};
}

DGLAdbDeviceProcess::DGLAdbDeviceProcess(const std::string& pid,
                                         const std::string& name,
                                         const std::string& portName)
        : m_Pid(pid), m_Name(name), m_PortName(portName) {}

bool DGLAdbDeviceProcess::operator<(const DGLAdbDeviceProcess& other) const {
    if (m_Name < other.getName()) {
        return true;
    } else if (m_Name == other.getName()) {
        return m_Pid < other.getPid();
    }
    return false;
}

const std::string& DGLAdbDeviceProcess::getPid() const { return m_Pid; }

const std::string& DGLAdbDeviceProcess::getName() const { return m_Name; }

const std::string& DGLAdbDeviceProcess::getPortName() const {
    return m_PortName;
}

DGLADBDevice::DGLADBDevice(const std::string& serial)
        : m_Serial(serial),
          m_Status(InstallStatus::UNKNOWN),
          m_RequestStatus(RequestStatus::IDLE),
          m_DetailRequestStatus(DetailRequestStatus::NONE),
          m_RootSuRequired(false) {}

void DGLADBDevice::reloadProcesses() {
    if (m_RequestStatus == RequestStatus::IDLE) {
        setRequestStatus(RequestStatus::RELOAD_PROCESSES_GET_PORTSTR);
        getProp("debug." DGL_PRODUCT_LOWER ".socket")->process();
    }
}

const std::string& DGLADBDevice::getSerial() const { return m_Serial; }

void DGLADBDevice::queryStatus() {
    if (!checkIdle()) {
        return;
    }
    setRequestStatus(RequestStatus::QUERY_INSTALL_STATUS);

    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("ls");
    params.push_back("/system/bin/app_process.dgl");

    invokeAsShellUser(params)->process();
}

DGLAdbCookie* DGLADBDevice::getProp(std::string prop) {
    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("getprop");
    params.push_back(prop);
    DGLAdbCookie* cookie = invokeAsShellUser(
            params, std::make_shared<DGLGetPropOutputFilter>());
    return cookie;
}

void DGLADBDevice::doneQueryInstallStatus(
        const std::vector<std::string>& prop) {

    setRequestStatus(RequestStatus::QUERY_ABI);

    if (prop.size() > 0 &&
        prop[0].find("/system/bin/app_process.dgl") != std::string::npos &&
        prop[0].find("No such file") == std::string::npos) {
        m_Status = InstallStatus::INSTALLED;
    } else {
        m_Status = InstallStatus::CLEAN;
    }

    getProp("ro.product.cpu.abi")->process();
}

void DGLADBDevice::doneQueryABI(const std::vector<std::string>& prop) {

    setRequestStatus(RequestStatus::IDLE);

    if (prop.size()) {
        if (prop[0].find("armeabi") != std::string::npos) {
            m_ABI = ABI::ARMEABI;
        }
        if (prop[0].find("x86") != std::string::npos) {
            m_ABI = ABI::X86;
        }
        if (prop[0].find("mips") != std::string::npos) {
            m_ABI = ABI::MIPS;
        }
    } else {
        m_ABI = ABI::UNKNOWN;
    }
    emit queryStatusSuccess(this);
}

void DGLADBDevice::portForward(std::string from, unsigned short to) {

    std::vector<std::string> params;
    params.push_back("forward");
    {
        std::ostringstream str;
        str << "tcp:" << to;
        params.push_back(str.str());
    }
    params.push_back("localfilesystem:" + from);

    // DGLAdbCookie* cookie = DGLAdbInterface::get()->invokeOnDevice(
    //        m_Serial, params, nullptr,
    // std::make_shared<DGLEmptyOutputFilter>());
    // CONNASSERT(cookie, SIGNAL(failed(std::string)), this,
    //           SLOT(adbFailed(std::string)));
    // CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this,
    //           SIGNAL(portForwardSuccess()));
    // cookie->process();
}

DGLADBDevice::InstallStatus DGLADBDevice::getInstallStatus() {
    return m_Status;
}

DGLADBDevice::ABI DGLADBDevice::getABI() { return m_ABI; }

void DGLADBDevice::reloadProcessesGotPortString(
        const std::vector<std::string>& prop) {

    setRequestStatus(RequestStatus::RELOAD_PROCESSES_GET_UNIXSOCKETS);

    const std::string& portString = prop[0];

    if (!portString.size()) {
        return;
    }

    QString pathRegexStr = "^";
    size_t lastOffset = 0;
    int currentRegexGroup = 0;

    m_PidInSocketRegex = -1;
    m_PNameInSocketRegex = -1;

    for (size_t i = 0; i < portString.length(); i++) {
        if (portString[i] == '%' && i < (portString.length() + 1)) {
            std::string current = portString.substr(lastOffset, i);
            pathRegexStr += QRegExp::escape(QString::fromStdString(current));

            switch (portString[i + 1]) {
                case 'p':
                    m_PidInSocketRegex = currentRegexGroup;
                    break;
                case 'n':
                    m_PNameInSocketRegex = currentRegexGroup;
                    break;
                default:
                    assert(0);
            }

            i += 2;
            pathRegexStr += "(.*)";

            currentRegexGroup++;
            lastOffset = i;
        }
    }

    if (lastOffset < portString.length()) {
        std::string last = portString.substr(lastOffset);
        pathRegexStr += QRegExp::escape(QString::fromStdString(last));
    }

    m_SocketPathRegex = QRegExp(pathRegexStr + "$");

    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("cat");
    params.push_back("/proc/net/unix");
    invokeAsShellUser(params, std::make_shared<DGLUnixSocketsFilter>())
            ->process();
}

void DGLADBDevice::reloadProcessesGotUnixSockets(
        const std::vector<std::string>& sockets) {

    setRequestStatus(RequestStatus::IDLE);

    std::vector<DGLAdbDeviceProcess> processes;

    for (size_t i = 0; i < sockets.size(); i++) {
        if (m_SocketPathRegex.indexIn(QString::fromStdString(sockets[i])) !=
            -1) {
            std::string pid =
                    m_SocketPathRegex.cap(m_PidInSocketRegex + 1).toStdString();
            std::string processName;
            if (m_PNameInSocketRegex >= 0) {
                processName = m_SocketPathRegex.cap(m_PNameInSocketRegex + 1)
                                      .toStdString();
            }
            processes.push_back(
                    DGLAdbDeviceProcess(pid, processName, sockets[i]));
        }
    }
    emit gotProcesses(this, processes);
}

void DGLADBDevice::installWrapper(std::string path) {
    m_InstallerPath = path;
    if (!checkIdle()) {
        return;
    }
    setRequestStatus(RequestStatus::PREP_INSTALL);
    checkUser()->process();
}

void DGLADBDevice::uninstallWrapper(std::string path) {
    m_InstallerPath = path;
    if (!checkIdle()) {
        return;
    }
    setRequestStatus(RequestStatus::PREP_UNINSTALL);
    checkUser()->process();
}

void DGLADBDevice::updateWrapper(std::string path) {
    m_InstallerPath = path;
    if (!checkIdle()) {
        return;
    }
    setRequestStatus(RequestStatus::PREP_UPDATE);
    checkUser()->process();
}

DGLAdbCookie* DGLADBDevice::checkUser() {
    setRequestStatus(DetailRequestStatus::PREP_ADB_CHECKUSER);
    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("id");
    return invokeAsShellUser(params);
}

DGLAdbCookie* DGLADBDevice::remountFromAdb() {
    setRequestStatus(DetailRequestStatus::PREP_REMOUNT_FROM_ADB);
    std::vector<std::string> params(1, "remount");
    return invokeAsShellUser(params, std::make_shared<DGLEmptyOutputFilter>());
}

DGLAdbCookie* DGLADBDevice::remountFromShell() {
    setRequestStatus(DetailRequestStatus::PREP_REMOUNT_FROM_SHELL);
    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("mount");
    params.push_back("-o");
    params.push_back("remount,rw");
    params.push_back("/system");
    return invokeAsRoot(params, std::make_shared<DGLEmptyOutputFilter>());
}

void DGLADBDevice::done(const std::vector<std::string>& data) {
    for (size_t i = 0; i < data.size(); i++) {
        emit log(this, "< " + data[i]);
    }
    switch (m_RequestStatus) {
        case RequestStatus::IDLE:
            emit failed(this, "Spurious done() call.");
            break;
        case RequestStatus::QUERY_INSTALL_STATUS:
            doneQueryInstallStatus(data);
            break;
        case RequestStatus::QUERY_ABI:
            doneQueryABI(data);
            break;
        case RequestStatus::RELOAD_PROCESSES_GET_PORTSTR:
            reloadProcessesGotPortString(data);
            break;
        case RequestStatus::RELOAD_PROCESSES_GET_UNIXSOCKETS:
            reloadProcessesGotUnixSockets(data);
            break;

        case RequestStatus::PREP_INSTALL:
        case RequestStatus::PREP_UPDATE:
        case RequestStatus::PREP_UNINSTALL: {
            switch (m_DetailRequestStatus) {
                case DetailRequestStatus::PREP_ADB_CHECKUSER:
                    if (data[0].find("uid=0(root)") != std::string::npos) {
                        remountFromAdb()->process();
                    } else {
                        setRequestStatus(
                                DetailRequestStatus::PREP_ADB_CHECK_SU_USER);
                        m_RootSuRequired = true;
                        std::vector<std::string> params;
                        params.push_back("shell");
                        params.push_back("id");
                        invokeAsRoot(params)->process();
                    }
                    break;
                case DetailRequestStatus::PREP_ADB_CHECK_SU_USER:
                    if (data[0].find("uid=0(root)") != std::string::npos) {
                        remountFromAdb()->process();
                    } else {
                        failed("Cannot get root permissions. Is this device "
                               "rooted?");
                    }
                    break;
                case DetailRequestStatus::PREP_REMOUNT_FROM_ADB:
                    remountFromShell()->process();
                    break;
                case DetailRequestStatus::PREP_REMOUNT_FROM_SHELL:
                    setRequestStatus(DetailRequestStatus::PREP_FRAMEWORK_STOP);
                    {
                        std::vector<std::string> params;
                        params.push_back("shell");
                        params.push_back("stop");
                        invokeAsRoot(params,
                                     std::make_shared<DGLEmptyOutputFilter>())
                                ->process();
                    }
                    break;
                case DetailRequestStatus::PREP_FRAMEWORK_STOP:
                    setRequestStatus(DetailRequestStatus::
                                             PREP_FRAMEWORK_UPLOAD_INSTALLER);
                    {
                        std::vector<std::string> params;
                        params.push_back("push");
                        params.push_back(m_InstallerPath +
                                         "/dglandroidinstaller");
                        params.push_back("/data/local/tmp/");
                        invokeAsShellUser(params,
                                          std::make_shared<DGLTransferFilter>())
                                ->process();
                    }
                    break;
                case DetailRequestStatus::PREP_FRAMEWORK_UPLOAD_INSTALLER:
                    setRequestStatus(DetailRequestStatus::
                                             PREP_FRAMEWORK_CHMOD_INSTALLER);
                    {
                        std::vector<std::string> params;
                        params.push_back("shell");
                        params.push_back("chmod");
                        params.push_back("777");
                        params.push_back("/data/local/tmp/dglandroidinstaller");
                        invokeAsRoot(params,
                                     std::make_shared<DGLEmptyOutputFilter>())
                                ->process();
                    }
                    break;
                case DetailRequestStatus::PREP_FRAMEWORK_CHMOD_INSTALLER:
                    setRequestStatus(
                            DetailRequestStatus::PREP_FRAMEWORK_RUN_INSTALLER);
                    {
                        std::vector<std::string> params;
                        params.push_back("shell");
                        params.push_back("/data/local/tmp/dglandroidinstaller");

                        switch (m_RequestStatus) {
                            case RequestStatus::PREP_INSTALL:
                                params.push_back("install");
                                break;
                            case RequestStatus::PREP_UPDATE:
                                params.push_back("update");
                                break;
                            case RequestStatus::PREP_UNINSTALL:
                                params.push_back("uninstall");
                                break;
                        }
                        invokeAsRoot(params,
                                     std::make_shared<DGLInstallerFilter>())
                                ->process();
                    }
                    break;
                case DetailRequestStatus::PREP_FRAMEWORK_RUN_INSTALLER:
                    setRequestStatus(DetailRequestStatus::PREP_FRAMEWORK_START);
                    {
                        std::vector<std::string> params;
                        params.push_back("shell");
                        params.push_back("start");
                        invokeAsRoot(params,
                                     std::make_shared<DGLEmptyOutputFilter>())
                                ->process();
                    }
                    break;
                case DetailRequestStatus::PREP_FRAMEWORK_START:
                    setRequestStatus(DetailRequestStatus::NONE);
                    setRequestStatus(RequestStatus::IDLE);
                    emit installerDone(this);
                    break;
            }
        }
    }
}

void DGLADBDevice::failed(const std::string& reason) {
    emit log(this, "Failure: " + reason);
    switch (m_RequestStatus) {
        case RequestStatus::QUERY_INSTALL_STATUS:
        case RequestStatus::QUERY_ABI:
            setRequestStatus(RequestStatus::IDLE);
            if (reason.find("error: device unauthorized.") !=
                std::string::npos) {
                m_Status = InstallStatus::UNAUTHORIZED;
                emit queryStatusSuccess(this);
            } else {
                emit failed(this, reason);
            }
            break;
        case RequestStatus::PREP_INSTALL:
        case RequestStatus::PREP_UNINSTALL:
        case RequestStatus::PREP_UPDATE:
            if (m_DetailRequestStatus ==
                DetailRequestStatus::PREP_REMOUNT_FROM_ADB) {
                if (reason.find("Permission denied") != std::string::npos ||
                    reason.find("Operation not permitted")) {
                    // this error is acceptable, happens on production devices.
                    remountFromShell()->process();
                }
            } else {
                setRequestStatus(RequestStatus::IDLE);
                emit failed(this, reason);
            }
            break;
        default:
            setRequestStatus(RequestStatus::IDLE);
            emit failed(this, reason);
    }
}

DGLAdbCookie* DGLADBDevice::invokeAsShellUser(
        const std::vector<std::string>& params,
        std::shared_ptr<DGLAdbOutputFilter> filter) {

    std::string msg = "> adb ";
    for (size_t i = 0; i < params.size(); i++) {
        msg += params[i];
        if (i < params.size()) {
            msg += " ";
        }
    }
    emit log(this, msg);

    DGLAdbCookie* cookie = DGLAdbInterface::get()->invokeOnDevice(
            m_Serial, params, this, filter);
    return cookie;
}

DGLAdbCookie* DGLADBDevice::invokeAsRoot(
        const std::vector<std::string>& params,
        std::shared_ptr<DGLAdbOutputFilter> filter) {

    if (!m_RootSuRequired || params[0] != "shell") {
        return invokeAsShellUser(params, filter);
    } else {
        std::vector<std::string> rootParams(4);
        rootParams[0] = params[0];    // shell
        rootParams[1] = "su";
        rootParams[2] = "-c";
        for (size_t i = 1; i < params.size(); i++) {
            rootParams[3] += params[i];
            if (i < params.size()) {
                rootParams[3] += " ";
            }
        }
        return invokeAsShellUser(rootParams, filter);
    }
}

bool DGLADBDevice::checkIdle() {
    if (m_RequestStatus == RequestStatus::IDLE) {
        return true;
    }
    emit failed(
            "Device busy - device is currently processing some adb commands");
    return false;
}

const char* DGLADBDevice::toString(RequestStatus status) {
    switch (status) {
        case RequestStatus::IDLE:
            return "Idle";
        case RequestStatus::RELOAD_PROCESSES_GET_PORTSTR:
            return "Get debugging port string";
        case RequestStatus::RELOAD_PROCESSES_GET_UNIXSOCKETS:
            return "Get open debugging sockets";
        case RequestStatus::QUERY_ABI:
            return "Check ABI";
        case RequestStatus::QUERY_INSTALL_STATUS:
            return "Check debugger installation";
        case RequestStatus::PREP_INSTALL:
            return "Installing";
        case RequestStatus::PREP_UPDATE:
            return "Updating";
        case RequestStatus::PREP_UNINSTALL:
            return "Uninstalling";
    }
    return "Unknown";
}

const char* DGLADBDevice::toString(DetailRequestStatus detailStatus) {
    switch (detailStatus) {
        case DetailRequestStatus::NONE:
            return "None";
        case DetailRequestStatus::PREP_ADB_CHECKUSER:
            return "Check adb user";
        case DetailRequestStatus::PREP_ADB_CHECK_SU_USER:
            return "Check su user";
        case DetailRequestStatus::PREP_REMOUNT_FROM_ADB:
            return "Remounting storage via adb";
        case DetailRequestStatus::PREP_REMOUNT_FROM_SHELL:
            return "Remounting storage from shell";
        case DetailRequestStatus::PREP_FRAMEWORK_STOP:
            return "Stopping framework";
        case DetailRequestStatus::PREP_FRAMEWORK_UPLOAD_INSTALLER:
            return "Uploading installer";
        case DetailRequestStatus::PREP_FRAMEWORK_CHMOD_INSTALLER:
            return "Setting installer permissions";
        case DetailRequestStatus::PREP_FRAMEWORK_RUN_INSTALLER:
            return "Running installer";
        case DetailRequestStatus::PREP_FRAMEWORK_START:
            return "Starting framework";
    }
    return "Unknown";
}

void DGLADBDevice::setRequestStatus(RequestStatus newStatus) {
    if (newStatus != m_RequestStatus) {
        emit log(this, std::string("Status: [") + toString(m_RequestStatus) +
                               "] => [" + toString(newStatus) + "]");
        m_RequestStatus = newStatus;
    }
}
void DGLADBDevice::setRequestStatus(DetailRequestStatus newDetailStatus) {
    if (newDetailStatus != m_DetailRequestStatus) {
        emit log(this, std::string("Detail Status: (") +
                               toString(m_DetailRequestStatus) + ") => (" +
                               toString(newDetailStatus) + ")");
        m_DetailRequestStatus = newDetailStatus;
    }
}