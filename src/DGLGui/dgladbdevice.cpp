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
        : m_Serial(serial), m_Status(InstallStatus::UNKNOWN) {}

void DGLADBDevice::reloadProcesses() {
    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("getprop");
    params.push_back("debug.debugler.socket");
    DGLAdbCookie* cookie = DGLAdbInterface::get()->invokeOnDevice(
            m_Serial, params, std::make_shared<DGLGetPropOutputFilter>());
    CONNASSERT(cookie, SIGNAL(failed(std::string)), this,
               SLOT(adbFailed(std::string)));
    CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this,
               SLOT(reloadProcessesGotPortString(std::vector<std::string>)));
    cookie->process();
}

const std::string& DGLADBDevice::getSerial() const { return m_Serial; }

void DGLADBDevice::queryInstallStatus() {
    std::vector<std::string> params;
    params.push_back("shell");
    params.push_back("ls");
    params.push_back("/system/bin/app_process.dgl");

    DGLAdbCookie* cookie =
            DGLAdbInterface::get()->invokeOnDevice(m_Serial, params);
    CONNASSERT(cookie, SIGNAL(failed(std::string)), this,
               SLOT(adbFailed(std::string)));
    CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this,
               SLOT(doneQueryInstallStatus(std::vector<std::string>)));
    cookie->process();
}

void DGLADBDevice::doneQueryInstallStatus(
        const std::vector<std::string>& prop) {
    if (prop.size() > 0 &&
        prop[0].find("/system/bin/app_process.dgl") != std::string::npos &&
        prop[0].find("No such file") == std::string::npos) {
        m_Status = InstallStatus::INSTALLED;
    } else {
        m_Status = InstallStatus::CLEAN;
    }
    emit queryInstallStatusSuccess(this);
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

    DGLAdbCookie* cookie = DGLAdbInterface::get()->invokeOnDevice(
            m_Serial, params, std::make_shared<DGLEmptyOutputFilter>());
    CONNASSERT(cookie, SIGNAL(failed(std::string)), this,
               SLOT(adbFailed(std::string)));
    CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this,
               SIGNAL(portForwardSuccess()));
    cookie->process();
}

DGLADBDevice::InstallStatus DGLADBDevice::getInstallStatus() {
    return m_Status;
}

void DGLADBDevice::reloadProcessesGotPortString(
        const std::vector<std::string>& prop) {

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
    DGLAdbCookie* cookie = DGLAdbInterface::get()->invokeOnDevice(
            m_Serial, params, std::make_shared<DGLUnixSocketsFilter>());
    CONNASSERT(cookie, SIGNAL(failed(std::string)), this,
               SLOT(adbFailed(std::string)));
    CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this,
               SLOT(reloadProcessesGotUnixSockets(std::vector<std::string>)));
    cookie->process();
}

void DGLADBDevice::reloadProcessesGotUnixSockets(
        const std::vector<std::string>& sockets) {
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

void DGLADBDevice::adbFailed(std::string reason) { emit failed(this, reason); }
