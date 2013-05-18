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



#include "dglconnectdialog.h"

DGLConnectDialog::DGLConnectDialog() {
    m_ui.setupUi(this);
    m_ui.lineEdit_TcpPort->setValidator(&m_portValidator);
}

DGLConnectDialog::~DGLConnectDialog() {}

std::string DGLConnectDialog::getAddress() {
    return m_ui.lineEdit_IpAddress->text().toStdString();

}

std::string DGLConnectDialog::getPort()
{
    return m_ui.lineEdit_TcpPort->text().toStdString();
}
