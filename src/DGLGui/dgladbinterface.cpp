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

namespace {
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
}

DGLAdbHandler::~DGLAdbHandler() {
    std::set<DGLAdbCookie*>::iterator i = m_RefCookies.begin();
    while (i != m_RefCookies.end()) {
        delete *(i++);
    }
}

void DGLAdbHandler::refCookie(DGLAdbCookie* cookie) {
    m_RefCookies.insert(cookie);
}

void DGLAdbHandler::unrefCookie(DGLAdbCookie* cookie) {
    m_RefCookies.erase(cookie);
}

DGLAdbCookie::DGLAdbCookie(DGLAdbHandler* handler,
                           std::shared_ptr<DGLAdbOutputFilter> filter)
        : m_OutputFilter(filter), m_Handler(handler) {
    m_Handler->refCookie(this);
}

DGLAdbCookie::~DGLAdbCookie() { m_Handler->unrefCookie(this); }

void DGLAdbCookie::filterOutput(const std::vector<std::string>& lines) {
    if (m_OutputFilter.get()) {
        std::vector<std::string> filteredLines;
        if (m_OutputFilter->filter(lines, filteredLines)) {
            onDone(filteredLines);
        } else {
            std::ostringstream msg;
            msg << "Cannot parse adb output: " << std::endl;
            for (size_t i = 0; i < lines.size(); i++) {
                msg << lines[i] << std::endl;
            }
            onFailed(msg.str());
        }
    } else {
        onDone(lines);
    }
}

void DGLAdbCookie::onDone(std::vector<std::string> data) {
    m_Handler->done(data);
}

void DGLAdbCookie::onFailed(std::string reason) { m_Handler->failed(reason); }

DGLAdbCookieImpl::DGLAdbCookieImpl(const std::string& adbPath,
                                   const std::vector<std::string>& params,
                                   DGLAdbHandler* handler,
                                   std::shared_ptr<DGLAdbOutputFilter> filter)
        : DGLAdbCookie(handler, filter), m_adbPath(adbPath), m_params(params), m_Deleted(false) {

    m_process = new DGLBaseQTProcess();
    m_process->setParent(this);

    CONNASSERT(m_process->getProcess(),
               SIGNAL(finished(int, QProcess::ExitStatus)), this,
               SLOT(handleProcessFinished(int, QProcess::ExitStatus)));
    CONNASSERT(m_process->getProcess(), SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(handleProcessError(QProcess::ProcessError)));
}

void DGLAdbCookieImpl::process() {
    if (!m_adbPath.size()) {
        onFailed(tr(
                "ADB path is not set, go to Tools->Configuration->Android to "
                "set it.").toStdString());
    } else {
        m_process->run(m_adbPath, "", m_params,
                       m_OutputFilter.get() != nullptr);
    }
}

void DGLAdbCookieImpl::handleProcessError(QProcess::ProcessError) {
    onFailed(m_process->getProcess()->errorString().toStdString());
    if (!m_Deleted) {
        m_Deleted = true;
        disconnect();
        deleteLater();
    }
}

void DGLAdbCookieImpl::handleProcessFinished(int code,
                                             QProcess::ExitStatus status) {
    if (status == QProcess::NormalExit) {

        QByteArray qData = m_process->getProcess()->readAllStandardError();
        qData.append(m_process->getProcess()->readAllStandardOutput());
        QList<QByteArray> qLines = qData.split('\n');
        std::vector<std::string> lines;

        bool suException = false;

        foreach(QByteArray qLine, qLines) {
            // skip adb server startup messages.
            if (qLine[0] == '*') {
                if (qLine.contains("daemon not running")) {
                    continue;
                }
                if (qLine.contains("daemon started successfully")) {
                    continue;
                }
            }
            // skip ChainsDD su non-fatal exception ant it's stacktrace
            if (suException) {
                if (qLine.contains("at ") ||
                    qLine.contains("Can't connect to activity manager")) {
                    continue;
                }
                suException = false;
            } else {
                if (qLine.contains("Error type 2")) {
                    suException = true;
                    continue;
                }
            }

            lines.push_back(
                    QString(qLine.replace("\r", QByteArray())).toStdString());
        }
        if (code) {
            if (lines[0].find("error:") == 0) {
                onFailed(lines[0]);
            } else {
                std::ostringstream msg;
                msg << "Adb process exit code :" << code << ":" << std::endl;
                msg << QString(m_process->getProcess()->readAll())
                                .toStdString();
                onFailed(msg.str());
            }
        } else {
            // success
            filterOutput(lines);
        }
    } else {
        onFailed("ADB process crashed");
    }
    if (!m_Deleted) {
        m_Deleted = true;
        disconnect();
        deleteLater();
    }
}

DGLAdbCookieFactory::DGLAdbCookieFactory(const std::string adbPath)
        : m_adbPath(adbPath) {}

const std::string& DGLAdbCookieFactory::getAdbPath() { return m_adbPath; }

DGLAdbCookie* DGLAdbCookieFactory::CreateCookie(
        const std::vector<std::string>& params, DGLAdbHandler* handler,
        std::shared_ptr<DGLAdbOutputFilter> filter) {

    return new DGLAdbCookieImpl(m_adbPath, params, handler, filter);
}

DGLAdbInterface* DGLAdbInterface::get() {
    if (!s_self) {
        s_self = std::make_shared<DGLAdbInterface>();
    }
    return s_self.get();
}

void DGLAdbInterface::setAdbCookieFactory(
        std::shared_ptr<DGLAdbCookieFactoryBase> factory) {
    m_factory = factory;
}

const std::string DGLAdbInterface::getAdbPath() const {
    DGLAdbCookieFactory* factory =
            dynamic_cast<DGLAdbCookieFactory*>(m_factory.get());
    if (factory) {
        return factory->getAdbPath();
    }
    return "";
}

std::shared_ptr<DGLAdbInterface> DGLAdbInterface::s_self;

DGLAdbCookie* DGLAdbInterface::killServer(DGLAdbHandler* handler) {
    std::vector<std::string> params;
    params.push_back("kill-server");
    return invokeAdb(params, handler, std::make_shared<DGLEmptyOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::connect(const std::string& address,
                                       DGLAdbHandler* handler) {
    std::vector<std::string> params;
    params.push_back("connect");
    params.push_back(address);
    return invokeAdb(params, handler,
                     std::make_shared<DGLConnectOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::getDevices(DGLAdbHandler* handler) {
    std::vector<std::string> params(1, "devices");
    return invokeAdb(params, handler,
                     std::make_shared<DGLDeviceOutputFilter>());
}

DGLAdbCookie* DGLAdbInterface::invokeOnDevice(
        const std::string& serial, const std::vector<std::string>& params,
        DGLAdbHandler* handler, std::shared_ptr<DGLAdbOutputFilter> filter) {
    std::vector<std::string> deviceParams(2 + params.size());
    deviceParams[0] = "-s";
    deviceParams[1] = serial;
    std::copy(params.begin(), params.end(), deviceParams.begin() + 2);

    DGLAdbCookie* ret = m_factory->CreateCookie(deviceParams, handler, filter);

    return ret;
}

DGLAdbCookie* DGLAdbInterface::invokeAdb(
        const std::vector<std::string>& params, DGLAdbHandler* handler,
        std::shared_ptr<DGLAdbOutputFilter> filter) {
    DGLAdbCookie* ret = m_factory->CreateCookie(params, handler, filter);
    return ret;
}
