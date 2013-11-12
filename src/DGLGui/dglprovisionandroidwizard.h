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

#ifndef DGLPROVISIONANDROIDDIALOG_H
#define DGLPROVISIONANDROIDDIALOG_H

#include "dglqtgui.h"
#include <QWizard>
#include <QLabel>
#include <QCheckBox>

namespace dglAndroidWizard {

class DGLProvisionAndroidWizard : public QWizard {
    Q_OBJECT
   public:
    DGLProvisionAndroidWizard(QWidget *parent = 0);
    void accept();
};

class StateMachine {
   public:
    StateMachine();
    enum class State {
        BEGIN,
        ROOT,
        STOPPED,
        COPIED,
        APP_PROCESS_BACKUP,
        APP_PRORCESS_SCRIPT,
        STARTED,
        COMPLETE,
        ERRORED
    };

   private:
    State m_State;
};

namespace pages {
class IntroPage : public QWizardPage {
    Q_OBJECT
   public:
    IntroPage(QWidget *parent = 0);

   private:
    QLabel *label;
    QCheckBox *acceptBox;
};

class ProvisionPage : public QWizardPage {
    Q_OBJECT
   public:
    ProvisionPage(QWidget *parent = 0);

   private:
    StateMachine m_StateMachine;
};
}
}
#endif    // DGLPROVISIONANDROIDDIALOG_H