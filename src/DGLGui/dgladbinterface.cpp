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

#include <stdexcept>

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

void DGLAdbInterface::killServer() {
    std::vector<std::string> params; 
    params.push_back("kill-server");
    invokeAdb(params);
}

void DGLAdbInterface::connect(std::string address) {
    std::vector<std::string> params;
    params.push_back("connect");
    params.push_back(address);
    invokeAdb(params);
}

std::vector<DGLAdbDevice> DGLAdbInterface::getDevices() {
    throw std::runtime_error("unimplemented");
}


void DGLAdbInterface::invokeAdb(std::vector<std::string> params) {
    throw std::runtime_error("unimplemented");
}


std::shared_ptr<DGLAdbInterface> DGLAdbInterface::s_self;
