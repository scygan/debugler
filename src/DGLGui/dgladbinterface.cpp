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

#include "dgladbinterface.h"
#include <QMessageBox>
#include <stdexcept>

class DGLAdbOutputFilter {
   public:
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) = 0;
    virtual ~DGLAdbOutputFilter() {}
};

class DGLConnectOutputFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>&) override {

        const char* pattern = "connected to ";

        if (input.size() && input[0].substr(0, strlen(pattern)) == pattern) {
            return true;
        } else {
            return false;
        }
    }
};

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

class DGLDeviceOutputFilter : public DGLAdbOutputFilter {
    virtual bool filter(const std::vector<std::string>& input,
                        std::vector<std::string>& output) override {
        if (!input.size() || input[0] != "List of devices attached ") {
            return false;
        }

        for (size_t i = 1; i < input.size(); i++) {
            if (!input[i].size()) {
                continue;
            }
            size_t pos = input[i].find('\t');
            if (pos != std::string::npos && pos > 0) {
                output.push_back(input[i].substr(0, pos));
            } else {
                output.push_back(input[i]);
            }
        }
        return true;
    }
};

DGLAdbCookie::DGLAdbCookie(const std::string& adbPath,
                           const std::vector<std::string>& params,
                           std::shared_ptr<DGLAdbOutputFilter> filter)
        : m_adbPath(adbPath), m_params(params), m_OutputFilter(filter) {
    CONNASSERT(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
               SLOT(handleProcessFinished(int, QProcess::ExitStatus)));
    CONNASSERT(&m_process, SIGNAL(error(QProcess::ProcessError)), this,
               SLOT(handleProcessError(QProcess::ProcessError)));
}

void DGLAdbCookie::process() {
    if (!m_adbPath.size()) {
        emit failed(
            tr("ADB path is not set, go to Tools->Configuration->Android to "
               "set it.").toStdString());
    } else {
        run(m_adbPath, "", m_params, m_OutputFilter.get() != nullptr);
    }
}

void DGLAdbCookie::handleProcessError(QProcess::ProcessError) {
    emit failed(m_process.errorString().toStdString());
    disconnect();
    deleteLater();
}

void DGLAdbCookie::handleProcessFinished(int code,
                                         QProcess::ExitStatus status) {
    if (status == QProcess::NormalExit) {
        if (code) {
            std::ostringstream msg;
            msg << "Adb process exit code :" << code << ":" << std::endl;
            msg << QString(m_process.readAll()).toStdString();
            emit failed(msg.str());
        } else {
            // success
            QByteArray qData = m_process.readAll();
            QList<QByteArray> qLines = qData.split('\n');
            std::vector<std::string> lines;
            foreach(QByteArray qLine, qLines) {
                if (qLine[0] != '*')
                    lines.push_back(QString(qLine.replace("\r", QByteArray()))
                                        .toStdString());
            }
            if (m_OutputFilter.get()) {
                std::vector<std::string> filteredLines;
                if (m_OutputFilter->filter(lines, filteredLines)) {
                    emit done(filteredLines);
                } else {
                    std::ostringstream msg;
                    msg << "Cannot parse adb output: " << std::endl;
                    for (size_t i = 0; i < lines.size(); i++) {
                        msg << lines[i] << std::endl;
                    }
                    emit failed(msg.str());
                }
            } else {
                emit done(lines);
            }
        }
    } else {
        emit failed("ADB process crashed");
        disconnect();
        deleteLater();
    }
}

DGLAdbProcess::DGLAdbProcess(const std::string& pid, const std::string& name,
                             const std::string& portName)
        : m_Pid(pid), m_Name(name), m_PortName(portName) {}

bool DGLAdbProcess::operator<(const DGLAdbProcess& other) {
    if (m_Name < other.getName()) {
        return true;
    } else if (m_Name == other.getName()) {
        return m_Pid < other.getPid();
    }
    return false;
}

const std::string& DGLAdbProcess::getPid() const { return m_Pid; }

const std::string& DGLAdbProcess::getName() const { return m_Name; }

const std::string& DGLAdbProcess::getPortName() const { return m_PortName; }

DGLADBDevice::DGLADBDevice(const std::string& serial) : m_Serial(serial) {}

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
    std::vector<DGLAdbProcess> processes;

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
            processes.push_back(DGLAdbProcess(pid, processName, sockets[i]));
        }
    }
    emit gotProcesses(processes);
}

void DGLADBDevice::adbFailed(std::string reason) { emit failed(this, reason); }

DGLAdbInterface* DGLAdbInterface::get() {
    if (!s_self) {
        s_self = std::make_shared<DGLAdbInterface>();
    }
    return s_self.get();
}

void DGLAdbInterface::setAdbPath(const std::string& path) { m_adbPath = path; }

const std::string& DGLAdbInterface::getAdbPath() const { return m_adbPath; }

DGLAdbCookie* DGLAdbInterface::killServer() {
    std::vector<std::string> params;
    params.push_back("kill-server");
    return invokeAdb(params, std::make_shared<DGLEmptyOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::connect(const std::string& address) {
    std::vector<std::string> params;
    params.push_back("connect");
    params.push_back(address);
    return invokeAdb(params, std::make_shared<DGLConnectOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::getDevices() {
    std::vector<std::string> params(1, "devices");
    return invokeAdb(params, std::make_shared<DGLDeviceOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::invokeOnDevice(
    const std::string& serial, const std::vector<std::string>& params,
    std::shared_ptr<DGLAdbOutputFilter> filter) {
    std::vector<std::string> deviceParams(2 + params.size());
    deviceParams[0] = "-s";
    deviceParams[1] = serial;
    std::copy(params.begin(), params.end(), deviceParams.begin() + 2);

    DGLAdbCookie* ret = new DGLAdbCookie(m_adbPath, deviceParams, filter);
    ;
    return ret;
}

DGLAdbCookie* DGLAdbInterface::invokeAdb(
    const std::vector<std::string>& params,
    std::shared_ptr<DGLAdbOutputFilter> filter) {
    DGLAdbCookie* ret = new DGLAdbCookie(m_adbPath, params, filter);
    ;
    return ret;
}

std::shared_ptr<DGLAdbInterface> DGLAdbInterface::s_self;
