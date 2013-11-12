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
    return Wizard::Page_Run;
}

DeviceChoice::DeviceChoice(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;

    m_SelectWidget = new DGLAndroidSelectDevWidget(this);

    //CONNASSERT(selectWidget, SIGNAL(selectDevice(DGLADBDevice*)), this,
    //    SLOT(selectDevice(DGLADBDevice*)));
    CONNASSERT(m_SelectWidget, SIGNAL(adbFailed(std::string)), this,
        SLOT(adbFailed(std::string)));

    layout->addWidget(m_SelectWidget);
    setLayout(layout);
}

void DeviceChoice::adbFailed(std::string reason) {
    QMessageBox::critical(this, tr("ADB Error"),
        QString::fromStdString(reason));
}

int DeviceChoice::nextId() const {
    if (m_SelectWidget->getCurrentDevice()) {
        return Wizard::Page_Run;
    }
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