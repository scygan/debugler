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

#include "dglprepareandroidwizard.h"

#include <QMessageBox>

#include "dglandroidselectdev.h"

namespace dglPrepareAndroidWizard {

namespace pages {
Intro::Intro(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Introduction"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/res/android.png"));

    label = new QLabel(tr(
            "<p>This wizard will install or uninstall debugging stubs on "
            "Android "
            "device.</p>"
            "<p>Only <b>development</b>,  <b>eng</b>,  <b>debug</b> or  "
            "<b>userdebug</b> AOSP builds are supported. <b>user</b> builds "
            "should "
            "be manually rooted before using this wizard.</p>"
            "<p>Your device may misbehave after this, including boot problems "
            "or "
            "soft-brick. Re-flash your device in case of serious problems.</p>"
            "<p>This software is distributed on an \"AS IS\" BASIS, WITHOUT "
            "WARRANTIES OR CONDITIONS OF ANY KIND.</p>"
            "<p>You have been warned!</p><br>"));
    label->setWordWrap(true);

    fakeAcceptBox = new QCheckBox("I agree.");
    acceptBox = new QCheckBox("I have understood and I agree to above.");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(fakeAcceptBox);
    layout->addWidget(acceptBox);
    setLayout(layout);
}

int Intro::nextId(void) const {
    if (!acceptBox->isChecked()) {
        return Wizard::Page_Intro;
    }
    return Wizard::Page_DeviceChoice;
}

DeviceChoice::DeviceChoice(QWidget *parent) : QWizardPage(parent), m_Device(nullptr) {
    QVBoxLayout *layout = new QVBoxLayout;

    m_SelectWidget = new DGLAndroidSelectDevWidget(this);

    CONNASSERT(m_SelectWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
        SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_SelectWidget, SIGNAL(adbFailed(std::string)), this,
        SLOT(adbFailed(std::string)));

    layout->addWidget(m_SelectWidget);
    m_DeviceStatusLabel = new QLabel();
    layout->addWidget(m_DeviceStatusLabel);

    
    m_RadioButtonClean = new QRadioButton("Uninstall debugler from device.");
    m_RadioButtonUpdate = new QRadioButton("Update debugler on device.");
    m_RadioButtonInstall = new QRadioButton("Install debugler on device.");
    QButtonGroup *bg = new QButtonGroup();
    bg->addButton(m_RadioButtonClean);
    bg->addButton(m_RadioButtonUpdate);
    bg->addButton(m_RadioButtonInstall);

    layout->addWidget(m_RadioButtonClean);
    layout->addWidget(m_RadioButtonUpdate);
    layout->addWidget(m_RadioButtonInstall);

    setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN);

    setLayout(layout);
}

void DeviceChoice::setDeviceStatus(DGLADBDevice::InstallStatus status) {
    if (status == DGLADBDevice::InstallStatus::UNKNOWN) {
            m_RadioButtonClean  ->setChecked(false);
            m_RadioButtonUpdate ->setChecked(false);
            m_RadioButtonInstall->setChecked(false);
            m_RadioButtonClean   -> setEnabled(false);
            m_RadioButtonUpdate  -> setEnabled(false);
            m_RadioButtonInstall -> setEnabled(false);
            m_DeviceStatusLabel->setText("Device status: unknown");
    }
    if (status == DGLADBDevice::InstallStatus::INSTALLED) {
        m_RadioButtonInstall->setChecked(false);
        m_RadioButtonClean   -> setEnabled(true);
        m_RadioButtonUpdate  -> setEnabled(true);
        m_RadioButtonInstall -> setEnabled(false);
        m_DeviceStatusLabel->setText("Device status: debugler is installed");
    }
    if (status == DGLADBDevice::InstallStatus::CLEAN) {
        m_RadioButtonClean  ->setChecked(false);
        m_RadioButtonUpdate ->setChecked(false);
        m_RadioButtonClean   -> setEnabled(false);
        m_RadioButtonUpdate  -> setEnabled(false);
        m_RadioButtonInstall -> setEnabled(true);
        m_DeviceStatusLabel->setText("Device status: debugler is not installed");
    }
}

void DeviceChoice::adbFailed(std::string reason) {
    QMessageBox::critical(this, tr("ADB Error"),
        QString::fromStdString(reason));
    setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN);
}

void DeviceChoice::selectDevice(DGLADBDevice* device) {
    if (device) {

        if (device != m_Device) {
            setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN);
            CONNASSERT(device, SIGNAL(queryInstallStatusSuccess(DGLADBDevice*)), this,
                SLOT(selectDeviceStatusSuccess(DGLADBDevice*)));
            
            m_Device = device;
            m_Device->queryInstallStatus();
        }        
    } else {
        setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN);
    }
}

void DeviceChoice::selectDeviceStatusSuccess(DGLADBDevice* device) {
    if (device == m_Device) {
        setDeviceStatus(m_Device->getInstallStatus());
    }
}

int DeviceChoice::nextId() {
    if (m_Device) {
        if (m_Device->getInstallStatus() == DGLADBDevice::InstallStatus::UNKNOWN) {
            QMessageBox::warning(this, tr("Device error"), tr("Device debugler install status is unknown."));
            return Wizard::Page_DeviceChoice;
        }
        if (m_Device->getInstallStatus() == DGLADBDevice::InstallStatus::INSTALLED) {
            if (m_RadioButtonClean->isChecked() || m_RadioButtonUpdate->isChecked()) {
                return Wizard::Page_Run;
            }
            QMessageBox::warning(this, tr("Selection error"), tr("Select on of available options."));
        }
        if (m_Device->getInstallStatus() == DGLADBDevice::InstallStatus::CLEAN) {
            if (m_RadioButtonInstall->isChecked()) {
                return Wizard::Page_Run;
            }
            QMessageBox::warning(this, tr("Selection error"), tr("Select on of available options."));
        }
    }
    QMessageBox::critical(this, tr("No device"),
        tr("No device selected. Please select appropriate device."));
    return Wizard::Page_DeviceChoice;
}

Run::Run(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget();
    setLayout(layout);
}

Conclusion::Conclusion(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget();
    setLayout(layout);
}
}

Wizard::Wizard(QWidget *parent) : QWizard(parent) {

    setPage(Page_Intro, new pages::Intro);
    setPage(Page_DeviceChoice, new pages::DeviceChoice);
    setPage(Page_Run, new pages::Run);
    setPage(Page_Conclusion, new pages::Conclusion);

    // setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
    // setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

    setWindowTitle(tr("Android Prepare Wizard"));
    setWizardStyle(QWizard::ModernStyle);
}

void Wizard::accept() {
    // QByteArray className = field("className").toByteArray();
    // QByteArray baseClass = field("baseClass").toByteArray();
    // QByteArray macroName = field("macroName").toByteArray();
    // QByteArray baseInclude = field("baseInclude").toByteArray();
    //
    // QString outputDir = field("outputDir").toString();
    // QString header = field("header").toString();
    // QString implementation = field("implementation").toString();
    //...
    QDialog::accept();
}
}