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

DGLConnectAndroidDialog::DGLConnectAndroidDialog() {
    
    m_ui.setupUi(this);
       
}

DGLConnectAndroidDialog::~DGLConnectAndroidDialog() {}

void DGLConnectAndroidDialog::adbKillServer() {

    try {
        DGLAdbInterface::get()->killServer();
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, tr("Adb kill-server error"), QString(e.what()));
    }
    
    reloadDevices();
}

void DGLConnectAndroidDialog::adbConnect() {

    if (m_ConnectDialog.exec() == QDialog::Accepted) {
        
        try {
            DGLAdbInterface::get()->connect(m_ConnectDialog.getAddress());
        } catch (const std::runtime_error& e) {
            QMessageBox::critical(this, tr("Adb connect error"), QString(e.what()));
        }

        reloadDevices();
    }
}


void DGLConnectAndroidDialog::reloadDevices() {
    throw std::runtime_error("unimplemented");
}


DGLConnectAndroidAdbDialog::DGLConnectAndroidAdbDialog() {
    m_ui.setupUi(this);
}

DGLConnectAndroidAdbDialog::~DGLConnectAndroidAdbDialog() {}

std::string DGLConnectAndroidAdbDialog::getAddress() {
    return m_ui.lineEdit_DeviceAddress->text().toStdString();
}