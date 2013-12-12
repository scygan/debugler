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

#include "dglconnectandroiddialog.h"

#include "dgladbinterface.h"
#include <QMessageBox>

#include <stdexcept>
#include <DGLCommon/def.h>

DGLConnectAndroidDialog::DGLConnectAndroidDialog() : m_Port(0) {
    m_ui.setupUi(this);

    CONNASSERT(m_ui.selectDevWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
               SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(update()), this, SLOT(update()));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(adbFailed(std::string)), this,
               SLOT(adbFailed(std::string)));
}

std::string DGLConnectAndroidDialog::getPort() {
    std::stringstream portStr;
    portStr << m_Port;
    return portStr.str();
}

void DGLConnectAndroidDialog::selectDevice(DGLADBDevice* device) {

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
        CONNASSERT(device, SIGNAL(portForwardSuccess(DGLADBDevice*)), this,
                   SLOT(portForwardSuccess(DGLADBDevice*)));
    }
}

void DGLConnectAndroidDialog::update() {
    if (m_ui.selectDevWidget->getCurrentDevice()) {
        m_ui.selectDevWidget->getCurrentDevice()->reloadProcesses();
    }
}

void DGLConnectAndroidDialog::gotProcesses(
        DGLADBDevice* device, std::vector<DGLAdbDeviceProcess> processes) {

    if (m_ui.selectDevWidget->getCurrentDevice() == device) {
        m_CurrentProcesses = processes;

        updateProcesses();
    }
}

void DGLConnectAndroidDialog::updateProcesses() {

    m_ui.label_deviceStatus->setText("ok.");

    std::sort(m_CurrentProcesses.begin(), m_CurrentProcesses.end());
    int j = 0;
    for (size_t i = 0; i < m_CurrentProcesses.size(); i++) {
        while (j < m_ui.comboBoxProcess->count() &&
               m_CurrentProcesses[i].getPid() <
                       m_ui.comboBoxProcess->itemText(j).toStdString()) {
            m_ui.comboBoxProcess->removeItem(j);
        }
        if (m_ui.comboBoxProcess->itemText(j).toStdString() !=
            m_CurrentProcesses[i].getPid()) {
            m_ui.comboBoxProcess->insertItem(
                    j, QIcon(),
                    QString::fromStdString(m_CurrentProcesses[i].getPid()));
        }
        j++;
    }
    while (m_ui.comboBoxProcess->count() > j) {
        m_ui.comboBoxProcess->removeItem(j);
    }
}

void DGLConnectAndroidDialog::tryAccept() {

    DGLADBDevice* device;

    if ((device = m_ui.selectDevWidget->getCurrentDevice()) == NULL) {
        QMessageBox::critical(this, tr("No device"),
                              tr("No device selected. Please select "
                                 "appropriate device and process."));
        return;
    }

    int idx;

    if (m_CurrentProcesses.size() == 0 ||
        (idx = m_ui.comboBoxProcess->currentIndex()) < 0) {
        QMessageBox::critical(this, tr("No process"),
                              tr("No process selected. Please select "
                                 "appropriate process running on device."));
        return;
    }

    m_Port = rand() % (0xffff - 1024) + 1024;

    device->portForward(m_CurrentProcesses[idx].getPortName(), m_Port);
}

void DGLConnectAndroidDialog::portForwardSuccess(DGLADBDevice* device) {

    if (m_ui.selectDevWidget->getCurrentDevice() == device) {
        accept();
    }
}

void DGLConnectAndroidDialog::deviceFailed(DGLADBDevice* device,
                                           std::string reason) {
    selectDevice(nullptr);

    QMessageBox::critical(this, tr("Device Error"),
                          QString::fromStdString(device->getSerial()) + ": " +
                                  QString::fromStdString(reason));
}

void DGLConnectAndroidDialog::adbFailed(std::string reason) {
    QMessageBox::critical(this, tr("ADB Error"),
                          QString::fromStdString(reason));
}