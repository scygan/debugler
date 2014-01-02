/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#ifndef DGLPROJECT_ANDROID_H
#define DGLPROJECT_ANDROID_H

#include <QWidget>

#include "ui_dglprojproperties_android.h"
#include "dgladbdevice.h"

class DGLAndroidProject : public DGLProject {
   public:
    DGLAndroidProject(std::string deviceSerial, std::string port);

   private:
    virtual void startDebugging() override;
    std::string m_deviceSerial;
    std::string m_processName;
};

class DGLAndroidProjectFactory : public DGLProjectFactory {
    Q_OBJECT
   public:
    DGLAndroidProjectFactory();

   private:
    virtual std::shared_ptr<DGLProject> createProject() override;

    virtual bool valid(QString&);
    virtual bool loadPropertiedFromProject(const DGLProject*);
    virtual QString getName() override;
    virtual QWidget* getGUI() override;

    void updateProcesses();

   private
slots:
    void selectDevice(DGLADBDevice*);
    void updateDialog();
    void adbFailed(std::string);
    void gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>);
    void deviceFailed(DGLADBDevice* device, std::string reason);

   private:
    Ui::DGLProjPropertiesAndroidClass m_ui;
    QWidget m_gui;

    std::vector<DGLAdbDeviceProcess> m_CurrentProcesses;
};

#endif