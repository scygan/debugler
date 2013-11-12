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

#include "dglprovisionandroidwizard.h"

//#include "dgladbinterface.h"

namespace dglAndroidWizard {
DGLProvisionAndroidWizard::DGLProvisionAndroidWizard(QWidget *parent)
        : QWizard(parent) {

    // addPage(new ClassInfoPage);
    // addPage(new CodeStylePage);
    // addPage(new OutputFilesPage);
    // addPage(new ConclusionPage);

    // setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
    // setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

    setWindowTitle(tr("Android Provision Wizard"));
}

void DGLProvisionAndroidWizard::accept() {
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

StateMachine::StateMachine() : m_State(State::BEGIN) {}

namespace pages {
IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Introduction"));
    // setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark1.png"));

    label = new QLabel(
        tr("This wizard will provision/un-provision a rooted Android device "
           "with OpenGL debugging stubs. \n\n"
           "Your device may misbehave after provisioning, including inability "
           "to boot without resetting to factory state/re-flashin operating "
           "system.\n "
           "Please take not that this software is distributed on an \"AS IS\" "
           "BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND.\n"
           "You have been warned!\n\n"
           "Only \"development\", \"eng\", \"debug\" or \"userdebug\" AOSP "
           "builds are supported. \"user\" builds should be manually rooted "
           "before using this wizard."
           "\n\n"));
    label->setWordWrap(true);

    acceptBox = new QCheckBox("I have read and understood above.");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(acceptBox);
    setLayout(layout);
}

ProvisionPage::ProvisionPage(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget();
    setLayout(layout);
}
}
}