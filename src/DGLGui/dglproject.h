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

#ifndef DGLPROJECT_H
#define DGLPROJECT_H

#include <QWidget>

class DGLProject {
public:
    virtual void startDebugging() = 0;
    ~DGLProject() {}
};


class DGLTCPProject: public DGLProject {
    virtual void startDebugging() override;
};


class DGLProjectFactory: QObject {
    Q_OBJECT
public:
    virtual QString getName() = 0;
    virtual QWidget* getGUI() = 0;
    virtual ~DGLProjectFactory() {}
};


#include "ui_dglprojproperties_tcpconn.h"


class DGLTCPConnectionProjectFactory: public DGLProjectFactory {
    virtual QString getName() override;
    virtual QWidget* getGUI() override;
    Ui::DGLProjPropertiesTCPConnClass m_ui;
};

#endif