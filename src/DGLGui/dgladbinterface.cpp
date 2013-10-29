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
    virtual std::vector<std::string> filter(std::vector<std::string>& in) = 0;
    virtual ~DGLAdbOutputFilter() {}
};

class DGLDeviceOutputFilter: public DGLAdbOutputFilter {
    std::vector<std::string> filter(std::vector<std::string>& in) {
        return in;
    }
};


DGLAdbCookie::DGLAdbCookie(const std::string& adbPath, const std::vector<std::string> params, 
                           std::shared_ptr<DGLAdbOutputFilter> filter):m_adbPath(adbPath), m_params(params), m_OutputFilter(filter) {
    CONNASSERT(connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleProcessFinished(int, QProcess::ExitStatus))));
    CONNASSERT(connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleProcessError(QProcess::ProcessError))));
}

void DGLAdbCookie::process() {
    if (!m_adbPath.size()) {
        emit failed(tr("ADB path is not set, go to Tools->Configuration->Android to set it.").toStdString());
    } else {
        run(m_adbPath, "", m_params, m_OutputFilter.get() != nullptr);
    }
}

void DGLAdbCookie::handleProcessError(QProcess::ProcessError) {
    emit failed(m_process.errorString().toStdString());
    disconnect();
    deleteLater();
}

void DGLAdbCookie::handleProcessFinished(int code, QProcess::ExitStatus status) {
    if (status == QProcess::NormalExit) {
        if (code) {
            std::ostringstream msg;
            msg << "Adb process exit code:" << code << ".";
            emit failed(msg.str());
        } else {
            //success
            QByteArray qData = m_process.readAll();
            QList<QByteArray> qLines = qData.split('\n');
            std::vector<std::string> lines;
            foreach ( QByteArray qLine, qLines) {
                lines.push_back(QString(qLine.replace("\r", QByteArray())).toStdString());
            }
            if (m_OutputFilter.get()) {
                emit done(m_OutputFilter->filter(lines));
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

DGLAdbInterface* DGLAdbInterface::get() {
    if (!s_self) {
        s_self = std::make_shared<DGLAdbInterface>();
    }
    return s_self.get();
}

void DGLAdbInterface::setAdbPath(std::string path) {
    m_adbPath = path;
}

std::string DGLAdbInterface::getAdbPath() {
    return m_adbPath;
}

DGLAdbCookie* DGLAdbInterface::killServer() {
    std::vector<std::string> params; 
    params.push_back("kill-server");
    return invokeAdb(params);
}

DGLAdbCookie* DGLAdbInterface::connect(std::string address) {
    std::vector<std::string> params;
    params.push_back("connect");
    params.push_back(address);
    return invokeAdb(params);
}

DGLAdbCookie* DGLAdbInterface::getDevices() {
    std::vector<std::string> params(1, "devices");
    return invokeAdb(params);
}

DGLAdbCookie* DGLAdbInterface::invokeAdb(std::vector<std::string> params) {
    DGLAdbCookie* ret = new DGLAdbCookie(m_adbPath, params);;
    return ret;
}


std::shared_ptr<DGLAdbInterface> DGLAdbInterface::s_self;
