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

#ifndef DGLANDROIDSELECTDEV_H
#define DGLANDROIDSELECTDEV_H

#include "dglqtgui.h"

#include "ui_dglandroidselectdev.h"

#include "ui_dglconnectandroidadb.h"
#include "dgladbdevice.h"

class DGLConnectAndroidAdbDialog : public QDialog {
    Q_OBJECT

   public:
    DGLConnectAndroidAdbDialog();
    ~DGLConnectAndroidAdbDialog();
    std::string getAddress();

   private:
    Ui::DGLConnectAndroidAdbDialogClass m_ui;
};

class DGLAndroidSelectDevWidget : public QWidget {
    Q_OBJECT

   public:
    DGLAndroidSelectDevWidget(QWidget* parent);
    DGLADBDevice* getCurrentDevice();
   public
slots:
    void adbKillServer();
    void adbConnect();
    void selectDevice(const QString& serial);

    void gotDevices(std::vector<std::string> devices);

signals:
    void selectDevice(DGLADBDevice*);
    void updateWidget();
    void adbFailed(std::string reason);

   private
slots:

    void reloadDevices();

   private:
    virtual void hideEvent(QHideEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;

    QTimer m_ReloadTimer;

    DGLConnectAndroidAdbDialog m_ConnectDialog;

    std::shared_ptr<DGLADBDevice> m_CurrentDevice;

    Ui::DGLAndroidSelectDev m_ui;
};

#endif    // DGLCONNECTDIALOG_H