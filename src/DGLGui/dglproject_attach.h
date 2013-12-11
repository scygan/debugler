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

#ifndef DGLPROJECT_ATTACH_H
#define DGLPROJECT_ATTACH_H

#include <QWidget>

#include "ui_dglprojproperties_attach.h"




class DGLAttachProject: public DGLProject {
public:
    DGLAttachProject(std::string address, std::string port);
    const std::string& getAddress() const;
    const std::string& getPort() const;
private:
    virtual void startDebugging() override;
    std::string m_address, m_port;
};

class DGLAttachProjectFactory: public DGLProjectFactory {
public:
    DGLAttachProjectFactory();
private:

    virtual std::shared_ptr<DGLProject> createProject() override;

    virtual bool valid(QString&);
    virtual bool loadPropertiedFromProject(const DGLProject*);
    virtual QString getName() override;
    virtual QWidget* getGUI() override;
    Ui::DGLProjPropertiesAttachClass m_ui;
    QWidget m_gui;
};


#endif