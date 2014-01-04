/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include "dglproject_android.h"

#include <QMessageBox>

DGLAndroidProject::DGLAndroidProject(std::string deviceSerial, std::string processPort)
        : m_deviceSerial(deviceSerial), m_processPort(processPort), m_Device(nullptr) {}

DGLAndroidProject::~DGLAndroidProject() {
    if (m_Device) {
        m_Device->deleteLater();
    }
}

void DGLAndroidProject::startDebugging() {
    m_ForwardedPort = rand() % (0xffff - 1024) + 1024;

    m_Device = new DGLADBDevice(m_deviceSerial);

    m_Device->portForward(m_processPort, m_ForwardedPort);
    CONNASSERT(m_Device, SIGNAL(portForwardSuccess(DGLADBDevice*)), this,
        SLOT(portForwardSuccess(DGLADBDevice*)));
}

bool DGLAndroidProject::shouldTerminateOnStop() {
    return false;
}

void DGLAndroidProject::portForwardSuccess(DGLADBDevice* device) {
   if (device == m_Device) {
       std::ostringstream portStr; 
       portStr << m_ForwardedPort;
       emit debugStarted("127.0.0.1", portStr.str());
       m_Device->deleteLater();
       m_Device = NULL;
   }
}

void DGLAndroidProject::deviceFailed(DGLADBDevice* device, const std::string& message) {
    if (device == m_Device) {
        emit debugError(tr("Device Error"), QString::fromStdString(m_Device->getSerial()) + ": " +
            QString::fromStdString(message));
        m_Device->deleteLater();
        m_Device = NULL;
    }   
}

DGLAndroidProjectFactory::DGLAndroidProjectFactory() {
    m_ui.setupUi(&m_gui);

    CONNASSERT(m_ui.selectDevWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
        SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(updateWidget()), this, SLOT(updateDialog()));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(adbFailed(std::string)), this,
        SLOT(adbFailed(std::string)));
}


std::shared_ptr<DGLProject> DGLAndroidProjectFactory::createProject() {
    QString message;
    if (!valid(message)) {
        throw std::runtime_error(message.toStdString());
    } else {
        int idx = m_ui.comboBoxProcess->currentIndex();
        return std::make_shared<DGLAndroidProject>(m_ui.selectDevWidget->getCurrentDevice()->getSerial(), 
            m_CurrentProcesses[idx].getPortName());
    }
}

bool DGLAndroidProjectFactory::valid(QString& message) {
    if (!m_ui.selectDevWidget->getCurrentDevice()) {
        message =
                tr("No device selected. Please select desired device from the list. Use \"adb connect...\" to connect to networked adb devices.");
        return false;
    }
    int idx = m_ui.comboBoxProcess->currentIndex();
    if (m_CurrentProcesses.size() == 0 ||
        (idx = m_ui.comboBoxProcess->currentIndex()) < 0) {
        message =
            tr("o process selected. Please select "
            "appropriate process running on device.");
        return false;
    }
    return true;
}

bool DGLAndroidProjectFactory::loadPropertiesFromProject(
        const DGLProject* project) {
    const DGLAndroidProject* androidProject =
            dynamic_cast<const DGLAndroidProject*>(project);
    if (!androidProject) {
        return false;
    }
    //m_ui.lineEdit_IpAddress->setText(
    //        QString::fromStdString(tcpProject->getAddress()));
    //m_ui.lineEdit_TcpPort->setText(
    //        QString::fromStdString(tcpProject->getPort()));
    return false;
}

QString DGLAndroidProjectFactory::getName() {
    return tr("Android(R) Application");
}

QWidget* DGLAndroidProjectFactory::getGUI() { return &m_gui; }

void DGLAndroidProjectFactory::updateProcesses() {

    m_ui.label_deviceStatus->setText("ok.");

    std::sort(m_CurrentProcesses.begin(), m_CurrentProcesses.end());
    int j = 0;
    for (size_t i = 0; i < m_CurrentProcesses.size(); i++) {
        while (j < m_ui.comboBoxProcess->count() &&
            m_CurrentProcesses[i].getDescriptionStr() <
            m_ui.comboBoxProcess->itemText(j).toStdString()) {
                m_ui.comboBoxProcess->removeItem(j);
        }
        if (m_ui.comboBoxProcess->itemText(j).toStdString() !=
            m_CurrentProcesses[i].getDescriptionStr()) {
                m_ui.comboBoxProcess->insertItem(
                    j, QIcon(),
                    QString::fromStdString(m_CurrentProcesses[i].getDescriptionStr()));
        }
        j++;
    }
    while (m_ui.comboBoxProcess->count() > j) {
        m_ui.comboBoxProcess->removeItem(j);
    }
}


void DGLAndroidProjectFactory::selectDevice(DGLADBDevice* device) {

    m_CurrentProcesses.clear();
    updateProcesses();

    if (!device) {
        m_ui.label_deviceStatus->setText("No device selected.");

    } else {
        m_ui.label_deviceStatus->setText(
            "not prepared. Go to Tools-> Prepare Android device.");

        CONNASSERT(
            device,
            SIGNAL(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)),
            this,
            SLOT(gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>)));
        CONNASSERT(device, SIGNAL(failed(DGLADBDevice*, std::string)), this,
            SLOT(deviceFailed(DGLADBDevice*, std::string)));
    }
}

void DGLAndroidProjectFactory::updateDialog() {
    if (m_ui.selectDevWidget->getCurrentDevice()) {
        m_ui.selectDevWidget->getCurrentDevice()->reloadProcesses();
    }
}

void DGLAndroidProjectFactory::adbFailed(std::string reason) {
    QMessageBox::critical(&m_gui, tr("ADB Error"),
        QString::fromStdString(reason));
}

void DGLAndroidProjectFactory::gotProcesses(
    DGLADBDevice* device, std::vector<DGLAdbDeviceProcess> processes) {

        if (m_ui.selectDevWidget->getCurrentDevice() == device) {
            m_CurrentProcesses = processes;

            updateProcesses();
        }
}

void DGLAndroidProjectFactory::deviceFailed(DGLADBDevice* device, std::string reason) {
    selectDevice(nullptr);

    QMessageBox::critical(&m_gui, tr("Device Error"),
        QString::fromStdString(device->getSerial()) + ": " +
        QString::fromStdString(reason));
}