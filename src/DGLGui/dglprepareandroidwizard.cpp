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

//#include "dgladbinterface.h"

namespace dglPrepareAndroidWizard {

namespace pages {
Intro::Intro(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Introduction"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/res/android.png"));

    label = new QLabel(tr(
        "<p>This wizard will install or uninstall debugging stubs on Android "
        "device.</p>"
        "<p>Only <b>development</b>,  <b>eng</b>,  <b>debug</b> or  "
        "<b>userdebug</b> AOSP builds are supported. <b>user</b> builds should "
        "be manually rooted before using this wizard.</p>"
        "<p>Your device may misbehave after this, including boot problems or "
        "soft-brick. Re-flash your device in case of serious problems.</p>"
        "<p>This software is distributed on an \"AS IS\" BASIS, WITHOUT "
        "WARRANTIES OR CONDITIONS OF ANY KIND.</p>"
        "<p>You have been warned!</p><br>"));
    label->setWordWrap(true);

    fakeAcceptBox = new QCheckBox("I have read above.");
    acceptBox = new QCheckBox("I have read and understood above.");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(fakeAcceptBox);
    layout->addWidget(acceptBox);
    setLayout(layout);
}

int Intro::nextId(void) const {
    return Wizard::Page_Run;
}

DeviceChoice::DeviceChoice(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget();
    setLayout(layout);
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