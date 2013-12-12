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

#include "dgladbinterface.h"

#include "dglandroidselectdev.h"

class DGLConnectAndroidDialog : public QDialog {
    Q_OBJECT

   public:
    DGLConnectAndroidDialog();

    std::string getPort();

   public
slots:
    void selectDevice(DGLADBDevice*);
    void update();
    void adbFailed(std::string reason);
    void deviceFailed(DGLADBDevice* device, std::string reason);

    void gotProcesses(DGLADBDevice*, std::vector<DGLAdbProcess> devices);

    void tryAccept();

    void portForwardSuccess(DGLADBDevice*);

   private:

    void updateProcesses();

    unsigned short m_Port;

    std::vector<DGLAdbProcess>  m_CurrentProcesses;

    Ui::DGLConnectAndroidDialogClass m_ui;
};

#endif    // DGLCONNECTDIALOG_H