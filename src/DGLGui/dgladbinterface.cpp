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

DGLAdbCookie::DGLAdbCookie(const std::string& adbPath, const std::vector<std::string> params):m_adbPath(adbPath), m_params(params) {
    CONNASSERT(connect(this, SIGNAL(processEvent(bool, std::string)), this, SLOT(processEvent(bool, std::string))));
}

void DGLAdbCookie::process() {
    if (!m_adbPath.size()) {
        emit failed(tr("ADB path is not set, go to Tools->Configuration->Android to set it.").toStdString());
    } else {
        run(m_adbPath, "", m_params, true);
    }
}

void DGLAdbCookie::processEvent(bool ok, std::string errormsg) {
    if (!ok) {
        emit failed(errormsg);
        disconnect();
        deleteLater();
    } else {
        emit done(std::vector<std::string>());
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


DGLAdbCookie* DGLAdbInterface::invokeAdb(std::vector<std::string> params) {
    DGLAdbCookie* ret = new DGLAdbCookie(m_adbPath, params);;
    return ret;
}


std::shared_ptr<DGLAdbInterface> DGLAdbInterface::s_self;
