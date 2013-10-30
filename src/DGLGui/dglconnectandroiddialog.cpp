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
    m_ReloadTimer.setInterval(1000);


    CONNASSERT(&m_ReloadTimer, SIGNAL(timeout()), this, SLOT(reloadDevices()));
    CONNASSERT(this, SIGNAL(finished(int)), &m_ReloadTimer, SLOT(stop()));

}

DGLConnectAndroidDialog::~DGLConnectAndroidDialog() {}

void DGLConnectAndroidDialog::adbKillServer() {

    DGLAdbCookie* cookie = DGLAdbInterface::get()->killServer();
    CONNASSERT(cookie, SIGNAL(failed(std::string)), this, SLOT(adbFailed(std::string)));
    CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this, SLOT(reloadDevices()));
    cookie->process();
}

void DGLConnectAndroidDialog::adbConnect() {

    if (m_ConnectDialog.exec() == QDialog::Accepted) {
        DGLAdbCookie* cookie = DGLAdbInterface::get()->connect(m_ConnectDialog.getAddress());
        CONNASSERT(cookie, SIGNAL(failed(std::string)), this, SLOT(adbFailed(std::string)));
        CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this, SLOT(reloadDevices()));
        cookie->process();
    }
}

void DGLConnectAndroidDialog::reloadDevices() {
     
     m_ReloadTimer.start();

     DGLAdbCookie* cookie = DGLAdbInterface::get()->getDevices();
     CONNASSERT(cookie, SIGNAL(failed(std::string)), this, SLOT(adbFailed(std::string)));
     CONNASSERT(cookie, SIGNAL(done(std::vector<std::string>)), this, SLOT(gotDevices(std::vector<std::string>)));
     cookie->process();
}

void DGLConnectAndroidDialog::gotDevices(std::vector<std::string> devices) {
    
    std::sort(devices.begin(), devices.end());

    int j = 0;
    for (size_t i = 0; i < devices.size(); i++) {
        while (j < m_ui.comboBox->count() && devices[i] < m_ui.comboBox->itemText(j).toStdString()) {
            m_ui.comboBox->removeItem(j);
        }
        if (m_ui.comboBox->itemText(j).toStdString() != devices[i]) {
            m_ui.comboBox->insertItem(j, QIcon(), QString::fromStdString(devices[i]));
        }
        j++;
    }
}

void DGLConnectAndroidDialog::showEvent(QShowEvent * event) {
    reloadDevices();
    QDialog::showEvent(event);
}

void DGLConnectAndroidDialog::adbFailed(std::string reason) {
    m_ReloadTimer.stop();
    QMessageBox::critical(this, tr("ADB Error"), QString::fromStdString(reason));
}


DGLConnectAndroidAdbDialog::DGLConnectAndroidAdbDialog() {
    m_ui.setupUi(this);
}

DGLConnectAndroidAdbDialog::~DGLConnectAndroidAdbDialog() {}

std::string DGLConnectAndroidAdbDialog::getAddress() {
    return m_ui.lineEdit_DeviceAddress->text().toStdString();
}
