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

DGLConnectAndroidDialog::DGLConnectAndroidDialog() {
    m_ui.setupUi(this);

    CONNASSERT(m_ui.selectDevWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
               SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(update()), this, SLOT(update()));
    CONNASSERT(m_ui.selectDevWidget, SIGNAL(adbFailed(std::string)), this,
               SLOT(adbFailed(std::string)));
}

void DGLConnectAndroidDialog::selectDevice(DGLADBDevice* device) {

    m_ui.comboBox_2->clear();

    if (!device) {
        m_ui.label_deviceStatus->setText("No device selected.");

    } else {
        m_ui.label_deviceStatus->setText(
                "not prepared. Go to Tools-> Prepare Android device.");

        CONNASSERT(device, SIGNAL(gotProcesses(std::vector<DGLAdbProcess>)),
                   this, SLOT(gotProcesses(std::vector<DGLAdbProcess>)));
        CONNASSERT(device, SIGNAL(failed(DGLADBDevice*, std::string)), this,
                   SLOT(deviceFailed(DGLADBDevice*, std::string)));
    }
}

void DGLConnectAndroidDialog::update() {
    if (m_ui.selectDevWidget->getCurrentDevice()) {
        m_ui.selectDevWidget->getCurrentDevice()->reloadProcesses();
    }
}

void DGLConnectAndroidDialog::gotProcesses(
        std::vector<DGLAdbProcess> processes) {

    m_ui.label_deviceStatus->setText("ok.");

    std::sort(processes.begin(), processes.end());
    int j = 0;
    for (size_t i = 0; i < processes.size(); i++) {
        while (j < m_ui.comboBox_2->count() &&
               processes[i].getPid() <
                       m_ui.comboBox_2->itemText(j).toStdString()) {
            m_ui.comboBox_2->removeItem(j);
        }
        if (m_ui.comboBox_2->itemText(j).toStdString() !=
            processes[i].getPid()) {
            m_ui.comboBox_2->insertItem(
                    j, QIcon(), QString::fromStdString(processes[i].getPid()));
        }
        j++;
    }
    while (m_ui.comboBox_2->count() > j) {
        m_ui.comboBox_2->removeItem(j);
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