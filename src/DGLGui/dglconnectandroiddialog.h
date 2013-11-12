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

#ifndef DGLCONNECTANDROIDDIALOG_H
#define DGLCONNECTANDROIDDIALOG_H

#include "dglqtgui.h"

#include "ui_dglconnectandroid.h"

#include "ui_dglconnectandroidadb.h"
#include "dgladbinterface.h"

class DGLConnectAndroidAdbDialog : public QDialog {
    Q_OBJECT

   public:
    DGLConnectAndroidAdbDialog();
    ~DGLConnectAndroidAdbDialog();
    std::string getAddress();

   private:
    Ui::DGLConnectAndroidAdbDialogClass m_ui;
};

class DGLConnectAndroidDialog : public QDialog {
    Q_OBJECT

   public:
    DGLConnectAndroidDialog();
    ~DGLConnectAndroidDialog();

   public
slots:
    void adbKillServer();
    void adbConnect();
    void selectDevice(const QString& serial);

    void deviceFailed(DGLADBDevice* device, std::string reason);
    void adbFailed(std::string reason);
    void gotDevices(std::vector<std::string> devices);
    void gotProcesses(std::vector<DGLAdbProcess> devices);

   private
slots:

    void reloadDevices();

   private:
    virtual void showEvent(QShowEvent* event) override;

    QTimer m_ReloadTimer;

    DGLConnectAndroidAdbDialog m_ConnectDialog;

    std::shared_ptr<DGLADBDevice> m_CurrentDevice;

    Ui::DGLConnectAndroidDialogClass m_ui;
};

#endif    // DGLCONNECTDIALOG_H