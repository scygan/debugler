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

namespace dglPrepareAndroidWizard {

class Wizard : public QWizard {
    Q_OBJECT
   public:
    Wizard(QWidget *parent = 0);
    void accept();

    enum {
        Page_Intro,
        Page_DeviceChoice,
        Page_Run,
        Page_Conclusion
    };
};

namespace pages {
class Intro : public QWizardPage {
    Q_OBJECT
   public:
    Intro(QWidget *parent = 0);

   private:
    int nextId() const;

    QLabel *label;
    QCheckBox *fakeAcceptBox;
    QCheckBox *acceptBox;
};

class DeviceChoice : public QWizardPage {
    Q_OBJECT
   public:
    DeviceChoice(QWidget *parent = 0);
};

class Run : public QWizardPage {
    Q_OBJECT
   public:
    Run(QWidget *parent = 0);
};

class Conclusion : public QWizardPage {
    Q_OBJECT
   public:
    Conclusion(QWidget *parent = 0);
};
}
}
#endif    // DGLPROVISIONANDROIDDIALOG_H