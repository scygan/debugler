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

#ifndef DGLCONFIGDIALOG_H
#define DGLCONFIGDIALOG_H

#include "dglqtgui.h"
#include "ui_dglconfigdialog.h"
#include "dglcontroller.h"

class DGLConfigDialog : public QDialog {
    Q_OBJECT

   public:
    DGLConfigDialog(const DGLConfiguration& configuration, std::string adbPath);
    ~DGLConfigDialog();

    const DGLConfiguration* getConfig();
    QString getAdbPath();

   public
slots:
    void toggleDebugFlagRenderingContext(bool);
    void adbBrowseDialog();

   private:
    Ui_ConfigDialog m_Ui;
    DGLConfiguration m_Configuration;
};

#endif    // DGLCONNECTDIALOG_H