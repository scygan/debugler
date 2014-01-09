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

#include "dglproject_attach.h"

#include <memory>
#include <stdexcept>

DGLAttachProject::DGLAttachProject(std::string address, std::string port)
        : m_address(address), m_port(port) {}

const std::string& DGLAttachProject::getAddress() const { return m_address; }

const std::string& DGLAttachProject::getPort() const { return m_port; }

void DGLAttachProject::startDebugging() {
    emit debugStarted(getAddress(), getPort());
}

bool DGLAttachProject::shouldTerminateOnStop() {
    return false;
}

DGLAttachProjectFactory::DGLAttachProjectFactory() { m_ui.setupUi(&m_gui); }

std::shared_ptr<DGLProject> DGLAttachProjectFactory::createProject() {
    QString message;
    if (!valid(message)) {
        throw std::runtime_error(message.toStdString());
    } else {
        return std::make_shared<DGLAttachProject>(
                m_ui.lineEdit_IpAddress->text().toStdString(),
                m_ui.lineEdit_TcpPort->text().toStdString());
    }
}

bool DGLAttachProjectFactory::valid(QString& message) {
    if (!m_ui.lineEdit_IpAddress->text().length()) {
        message =
                tr("No address set. Please file in network address (IP or "
                   "hostname) of target machine.");
        return false;
    }
    if (!m_ui.lineEdit_TcpPort->text().length()) {
        message =
                tr("No port set. Please file in tcp port on which debugee "
                   "process is listening.");
        return false;
    }
    return true;
}

bool DGLAttachProjectFactory::loadPropertiesFromProject(
        const DGLProject* project) {
    const DGLAttachProject* tcpProject =
            dynamic_cast<const DGLAttachProject*>(project);
    if (!tcpProject) {
        return false;
    } else {
        m_ui.lineEdit_IpAddress->setText(
                QString::fromStdString(tcpProject->getAddress()));
        m_ui.lineEdit_TcpPort->setText(
                QString::fromStdString(tcpProject->getPort()));
    }
    return true;
}

QString DGLAttachProjectFactory::getName() {
    return tr("Attach to Remote Application (DGLLoader/TCP)");
}

QWidget* DGLAttachProjectFactory::getGUI() { return &m_gui; }
