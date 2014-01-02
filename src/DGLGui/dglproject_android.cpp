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

DGLAndroidProject::DGLAndroidProject(std::string deviceSerial, std::string processName)
        : m_deviceSerial(deviceSerial), m_processName(processName) {}

void DGLAndroidProject::startDebugging() {
    
    emit debugStarted("127.0.0.1", "");
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
        return std::shared_ptr<DGLProject>();
        //return std::make_shared<DGLAndroidProject>(
        //        m_ui.lineEdit_IpAddress->text().toStdString(),
        //        m_ui.lineEdit_TcpPort->text().toStdString());
    }
}

bool DGLAndroidProjectFactory::valid(QString& message) {
    //if (!m_ui.lineEdit_IpAddress->text().length()) {
    //    message =
    //            tr("No address set. Please file in network address (IP or "
    //               "hostname) of target machine.");
    //    return false;
    //}
    //if (!m_ui.lineEdit_TcpPort->text().length()) {
    //    message =
    //            tr("No port set. Please file in tcp port on which debugee "
    //               "process is listening.");
    //    return false;
    //}
    return true;
}

bool DGLAndroidProjectFactory::loadPropertiedFromProject(
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