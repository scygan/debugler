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

#include <DGLCommon/def.h>

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
            "should be manually rooted before using this wizard.</p>"
            "<p>Rooted devices should use latest versions of <b>su</b> binary. "
            "<b>Chainfire SuperSU</b> is known to be problematic. Use "
            "<a href=\"https://play.google.com/store/apps/details?id=com.noshufou.android.su\">"
            " ChainsDD Superuser</a> instead.</p>"
            "<p>Your device may misbehave after this, including boot problems "
            "or soft-brick. Re-flash your device in case of serious"
            " problems.</p>"
            "<p>This software is distributed on an \"AS IS\" BASIS, WITHOUT "
            "WARRANTIES OR CONDITIONS OF ANY KIND.</p>"
            "<p>You have been warned!</p><br>"));
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);

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

DeviceChoice::DeviceChoice(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;

    setCommitPage(true);

    m_ReloadTimer.setInterval(1000);

    CONNASSERT(&m_ReloadTimer, SIGNAL(timeout()), this, SLOT(reloadStatus()));

    m_SelectWidget = new DGLAndroidSelectDevWidget(this);

    CONNASSERT(m_SelectWidget, SIGNAL(selectDevice(DGLADBDevice *)), this,
               SLOT(selectDevice(DGLADBDevice *)));
    CONNASSERT(m_SelectWidget, SIGNAL(adbFailed(std::string)), this,
               SLOT(adbFailed(std::string)));

    layout->addWidget(m_SelectWidget);
    m_DeviceStatusLabel = new QLabel();
    layout->addWidget(m_DeviceStatusLabel);

    m_DeviceABILabel = new QLabel();
    layout->addWidget(m_DeviceABILabel);

    m_RadioButtonClean =
            new QRadioButton("Uninstall " DGL_PRODUCT " from device.");
    m_RadioButtonUpdate = new QRadioButton("Update " DGL_PRODUCT " on device.");
    m_RadioButtonInstall =
            new QRadioButton("Install " DGL_PRODUCT " on device.");

    QButtonGroup *bg = new QButtonGroup();

    CONNASSERT(bg, SIGNAL(buttonClicked(int)), this, SIGNAL(completeChanged()));

    bg->addButton(m_RadioButtonClean);
    bg->addButton(m_RadioButtonUpdate);
    bg->addButton(m_RadioButtonInstall);

    layout->addWidget(m_RadioButtonClean);
    layout->addWidget(m_RadioButtonUpdate);
    layout->addWidget(m_RadioButtonInstall);

    registerField("clean-button", m_RadioButtonClean);
    registerField("update-button", m_RadioButtonUpdate);
    registerField("install-button", m_RadioButtonInstall);

    registerField("device*", this, "deviceProp", SIGNAL(completeChanged()));


    m_Status = DGLADBDevice::InstallStatus::UNKNOWN;
    m_Abi = DGLADBDevice::ABI::UNKNOWN;
    
    setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN,
                    DGLADBDevice::ABI::UNKNOWN, true);

    setLayout(layout);

    emit completeChanged();
}

DGLADBDevice *DeviceChoice::device() const {
    return m_SelectWidget->getCurrentDevice();
}

void DeviceChoice::setDeviceStatus(DGLADBDevice::InstallStatus status,
                                   DGLADBDevice::ABI abi, bool force) {

    if (m_Status ==  status && m_Abi == abi && !force) {
        return; 
    }

    m_Status = status;
    m_Abi = abi;

    m_ReloadTimer.stop();

    if (status == DGLADBDevice::InstallStatus::UNKNOWN ||
        status == DGLADBDevice::InstallStatus::UNAUTHORIZED) {
        m_RadioButtonClean->setChecked(false);
        m_RadioButtonUpdate->setChecked(false);
        m_RadioButtonInstall->setChecked(false);
        m_RadioButtonClean->setEnabled(false);
        m_RadioButtonUpdate->setEnabled(false);
        m_RadioButtonInstall->setEnabled(false);
        if (status == DGLADBDevice::InstallStatus::UNAUTHORIZED) {
            m_DeviceStatusLabel->setText(
                    "error: device unauthorized. Please check the confirmation "
                    "dialog on your device.");
        } else {
            m_DeviceStatusLabel->setText("Device status: unknown");
        }
    } else if (status == DGLADBDevice::InstallStatus::INSTALLED) {
        m_RadioButtonInstall->setChecked(false);
        m_RadioButtonClean->setEnabled(true);
        m_RadioButtonUpdate->setEnabled(true);
        m_RadioButtonInstall->setEnabled(false);
        m_DeviceStatusLabel->setText("Device status: " DGL_PRODUCT
                                     " is installed");
    } else if (status == DGLADBDevice::InstallStatus::CLEAN) {
        m_RadioButtonClean->setChecked(false);
        m_RadioButtonUpdate->setChecked(false);
        m_RadioButtonClean->setEnabled(false);
        m_RadioButtonUpdate->setEnabled(false);
        m_RadioButtonInstall->setEnabled(true);
        m_DeviceStatusLabel->setText("Device status: " DGL_PRODUCT
                                     " is not installed");
    }
    if (abi == DGLADBDevice::ABI::UNKNOWN) {
        m_DeviceABILabel->setText("Device abi: unrecognized");
    } else if (abi == DGLADBDevice::ABI::X86) {
        m_DeviceABILabel->setText("Device abi: x86 (supported)");
    } else if (abi == DGLADBDevice::ABI::ARMEABI) {
        m_DeviceABILabel->setText("Device abi: armeabi (supported)");
    } else if (abi == DGLADBDevice::ABI::MIPS) {
        m_DeviceABILabel->setText("Device abi: mips (supported)");
    }

    emit completeChanged();

}

void DeviceChoice::hideEvent(QHideEvent *event) {
    m_ReloadTimer.stop();
    QWidget::hideEvent(event);
}

void DeviceChoice::showEvent(QShowEvent *event) {
    m_ReloadTimer.start();
    QWidget::showEvent(event);
}

void DeviceChoice::adbFailed(std::string reason) {
    QMessageBox::critical(this, tr("ADB Error"),
                          QString::fromStdString(reason));

    setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN,
                    DGLADBDevice::ABI::UNKNOWN);
}

void DeviceChoice::selectDevice(DGLADBDevice *device) {
    if (device) {

        if (device == m_SelectWidget->getCurrentDevice()) {
            setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN,
                            DGLADBDevice::ABI::UNKNOWN);
            CONNASSERT(device, SIGNAL(queryStatusSuccess(DGLADBDevice *)), this,
                       SLOT(queryDeviceStatusSuccess(DGLADBDevice *)));

            reloadStatus();
        }
    } else {
        setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN,
                        DGLADBDevice::ABI::UNKNOWN);
    }

    emit completeChanged();
}

void DeviceChoice::reloadStatus() {
    if (m_SelectWidget->getCurrentDevice()) {
        m_SelectWidget->getCurrentDevice()->queryStatus();
    } else {
        setDeviceStatus(DGLADBDevice::InstallStatus::UNKNOWN,
                        DGLADBDevice::ABI::UNKNOWN);
    }
}

void DeviceChoice::queryDeviceStatusSuccess(DGLADBDevice *device) {
    if (device == m_SelectWidget->getCurrentDevice()) {
        setDeviceStatus(m_SelectWidget->getCurrentDevice()->getInstallStatus(),
                        m_SelectWidget->getCurrentDevice()->getABI());
    }
}

bool DeviceChoice::isComplete() const {
    if (m_SelectWidget->getCurrentDevice() &&
        m_Abi != DGLADBDevice::ABI::UNKNOWN) {
        if (m_Status == DGLADBDevice::InstallStatus::INSTALLED) {
            if (m_RadioButtonClean->isChecked() ||
                m_RadioButtonUpdate->isChecked()) {
                return true;
            }
        }
        if (m_Status == DGLADBDevice::InstallStatus::CLEAN) {
            if (m_RadioButtonInstall->isChecked()) {
                return true;
            }
        }
    }
    return false;
}

Run::Run(QWidget *parent)
        : QWizardPage(parent),
          m_Device(nullptr),
          m_Complete(false),
          m_Final(false) {
    QVBoxLayout *layout = new QVBoxLayout;
    m_LogWidget = new QListWidget;
    layout->addWidget(m_LogWidget);
    setLayout(layout);
}

void Run::initializePage() {
    m_LogWidget->clear();
    m_Device = field("device").value<DGLADBDevice *>();
    if (!m_Device) {
        QMessageBox::critical(this, tr("Installation error"),
                              tr("Device was not selected"));
        setFinalPage(true);
        m_Complete = true;
        m_Final = true;
    } else {

        CONNASSERT(m_Device, SIGNAL(installerDone(DGLADBDevice *)), this,
                   SLOT(installerDone(DGLADBDevice *)));
        CONNASSERT(m_Device, SIGNAL(failed(DGLADBDevice *, std::string)), this,
                   SLOT(failed(DGLADBDevice *, std::string)));
        CONNASSERT(m_Device, SIGNAL(log(DGLADBDevice *, std::string)), this,
                   SLOT(log(DGLADBDevice *, std::string)));

        m_Complete = false;
        m_Final = false;
        setFinalPage(false);

        QSettings settings(DGL_MANUFACTURER, DGL_PRODUCT);
#ifdef _WIN32
        std::string path =
                settings.value("InstallDir").toString().toStdString();
#else
       std::string path =  QFileInfo(QCoreApplication::applicationFilePath()).absoluteDir().path().toStdString() +
                "/../share/debugler";
#endif

        DGLADBDevice::ABI abi = m_Device->getABI();

        if (abi == DGLADBDevice::ABI::X86) {
            path += "/android-x86/";
        } else if (abi == DGLADBDevice::ABI::ARMEABI) {
            path += "/android-arm/";
        } else if (abi == DGLADBDevice::ABI::MIPS) {
            path += "/android-mips/";
        }

        if (!path.size()) {
            QMessageBox::critical(this, tr("Installation error"),
                                  tr("Cannot get installation path"));
            setFinalPage(true);
            m_Complete = true;
        } else {
            if (field("clean-button").value<bool>()) {
                m_Device->uninstallWrapper(path);
            }

            if (field("update-button").value<bool>()) {
                m_Device->updateWrapper(path);
            }

            if (field("install-button").value<bool>()) {
                m_Device->installWrapper(path);
            }
        }
    }
    emit completeChanged();
}

void Run::failed(DGLADBDevice *, const std::string &reason) {
    QMessageBox::critical(this, tr("Installation error"),
                          QString::fromStdString(reason));
    setFinalPage(true);
    m_Complete = true;
    m_Final = true;
    emit completeChanged();
}

void Run::installerDone(DGLADBDevice *) {
    m_Complete = true;
    m_Final = false;
    emit completeChanged();
    wizard()->next();
}

void Run::log(DGLADBDevice *, const std::string &log) {
    m_LogWidget->insertItem(m_LogWidget->count(), QString::fromStdString(log));
    m_LogWidget->scrollToBottom();
}

bool Run::isComplete() const { return m_Complete; }

int Run::nextId() const {
    if (m_Final) return -1;
    return Wizard::Page_Conclusion;
}

Conclusion::Conclusion(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Installer finished successfully."));
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

void Wizard::accept() { QDialog::accept(); }
}
