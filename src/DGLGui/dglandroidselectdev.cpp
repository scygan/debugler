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

#include "dglandroidselectdev.h"

DGLAndroidSelectDevWidget::DGLAndroidSelectDevWidget(QWidget* parent)
        : m_KillOrConnectHandler(this), QWidget(parent) {
    m_ui.setupUi(this);

    CONNASSERT(&m_ReloadTimer, SIGNAL(timeout()), this, SLOT(reloadDevices()));
}

DGLADBDevice* DGLAndroidSelectDevWidget::getCurrentDevice() {
    return m_CurrentDevice.get();
}

void DGLAndroidSelectDevWidget::setPreselectedDevice(const std::string& serial) {
    m_PreselectedDeviceSerial = serial;
}

void DGLAndroidSelectDevWidget::adbKillServer() {

    DGLAdbCookie* cookie = DGLAdbInterface::get()->killServer(&m_KillOrConnectHandler);
    cookie->process();
}

void DGLAndroidSelectDevWidget::adbConnect() {

    if (m_ConnectDialog.exec() == QDialog::Accepted) {
        DGLAdbCookie* cookie =
                DGLAdbInterface::get()->connect(m_ConnectDialog.getAddress(), &m_KillOrConnectHandler);
        cookie->process();
    }
}

void DGLAndroidSelectDevWidget::selectDevice(const QString& serial) {
    if (m_CurrentDevice.get() &&
        m_CurrentDevice->getSerial() != serial.toStdString()) {

        m_CurrentDevice->disconnect();
        m_CurrentDevice.reset();

        emit selectDevice(nullptr);
    }
    if (serial.size()) {
        m_CurrentDevice = std::make_shared<DGLADBDevice>(serial.toStdString());
        emit selectDevice(m_CurrentDevice.get());
    }
}

void DGLAndroidSelectDevWidget::reloadDevices() {

    m_ReloadTimer.setInterval(1000);
    m_ReloadTimer.start();

    DGLAdbCookie* cookie = DGLAdbInterface::get()->getDevices(this);

    cookie->process();
}

void DGLAndroidSelectDevWidget::gotDevices(std::vector<std::string> devices) {

    std::sort(devices.begin(), devices.end());

    if (m_PreselectedDeviceSerial.length()) {
        m_ui.comboBox->setCurrentIndex(-1);
    }

    int j = 0;
    for (size_t i = 0; i < devices.size(); i++) {
        while (j < m_ui.comboBox->count() &&
               devices[i] < m_ui.comboBox->itemText(j).toStdString()) {
            m_ui.comboBox->removeItem(j);
        }
        if (m_ui.comboBox->itemText(j).toStdString() != devices[i]) {
            m_ui.comboBox->insertItem(j, QIcon(),
                                      QString::fromStdString(devices[i]));
        }
        j++;
    }
    while (m_ui.comboBox->count() > j) {
        m_ui.comboBox->removeItem(j);
    }

    if (m_PreselectedDeviceSerial.length()) {

        std::vector<std::string>::iterator i =
            std::find(devices.begin(), devices.end(), m_PreselectedDeviceSerial);

        if (i == devices.end()) {
            std::string serial = "";
            std::swap(m_PreselectedDeviceSerial, serial);
            emit adbFailed("Device not found: " + serial);
        } else {
            m_ui.comboBox->setCurrentIndex(i - devices.begin());
        }

        m_PreselectedDeviceSerial = "";
    }

    if (m_CurrentDevice.get()) {
        emit updateWidget();
    }
}

void DGLAndroidSelectDevWidget::showEvent(QShowEvent* event) {
    if (!m_ReloadTimer.isActive()) {
        m_ReloadTimer.setInterval(1);
        m_ReloadTimer.start();
    }
    QWidget::showEvent(event);
}

void DGLAndroidSelectDevWidget::hideEvent(QHideEvent* event) {
    m_ReloadTimer.stop();
    QWidget::hideEvent(event);
}

void DGLAndroidSelectDevWidget::KillOrConnectHandler::done(const std::vector<std::string>&) {
    m_Parent->reloadDevices();
}

void DGLAndroidSelectDevWidget::KillOrConnectHandler::failed(const std::string& reason) {
    emit m_Parent->adbFailed(reason);
}

void DGLAndroidSelectDevWidget::done(const std::vector<std::string>& data) {
    gotDevices(data);
}

void DGLAndroidSelectDevWidget::failed(const std::string& reason) {
    m_ReloadTimer.stop();
    emit adbFailed(reason);
}

DGLConnectAndroidAdbDialog::DGLConnectAndroidAdbDialog() { m_ui.setupUi(this); }

DGLConnectAndroidAdbDialog::~DGLConnectAndroidAdbDialog() {}

std::string DGLConnectAndroidAdbDialog::getAddress() {
    return m_ui.lineEdit_DeviceAddress->text().toStdString();
}