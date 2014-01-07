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
   Q_OBJECT
   public:
    DGLAndroidProject(const std::string& deviceSerial, const std::string& processName, const std::string& pid = "");

    ~DGLAndroidProject();

    const std::string& getDeviceSerial() const;
    const std::string& getPid() const;
    const std::string& getProcessName() const;


   private slots:
    void portForwardSuccess(DGLADBDevice*);
    void setProcessBreakPointSuccess(DGLADBDevice*);
    void deviceFailed(DGLADBDevice*, const std::string&);
    void gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>);
   private:
    virtual void startDebugging() override;
    virtual void stopDebugging() override;
    virtual bool shouldTerminateOnStop() override;
    std::string m_deviceSerial;
    std::string m_processName;
    std::string m_pid;

    unsigned short m_ForwardedPort;
    DGLADBDevice* m_Device;
};

class DGLAndroidProjectFactory : public DGLProjectFactory {
    Q_OBJECT
   public:
    DGLAndroidProjectFactory();

   private:
    virtual std::shared_ptr<DGLProject> createProject() override;

    virtual bool valid(QString&);
    virtual bool loadPropertiesFromProject(const DGLProject*);
    virtual QString getName() override;
    virtual QWidget* getGUI() override;

    void updateProcesses();
    void updatePackages();

   private
slots:
    void selectDevice(DGLADBDevice*);
    void radioStartupChanged(bool);
    void updateDialog();
    void adbFailed(std::string);
    void gotProcesses(DGLADBDevice*, std::vector<DGLAdbDeviceProcess>);
    void gotPackages(DGLADBDevice*, std::vector<std::string>);
    void deviceFailed(DGLADBDevice* device, std::string reason);

   private:
    Ui::DGLProjPropertiesAndroidClass m_ui;
    QWidget m_gui;

    std::vector<DGLAdbDeviceProcess> m_CurrentProcesses;
    std::shared_ptr<DGLAdbDeviceProcess> m_PreselectedProcess;

    std::vector<std::string> m_CurrentPackages;

    bool m_Attach;
};

#endif