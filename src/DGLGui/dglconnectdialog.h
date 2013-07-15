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


#ifndef DGLCONNECTDIALOG_H
#define DGLCONNECTDIALOG_H

#include "dglqtgui.h"
#include <QIntValidator>
#include "ui_dglconnectdialog.h"

class DGLConnectDialog : public QDialog
{
    Q_OBJECT

public:
    DGLConnectDialog();
    ~DGLConnectDialog();
    std::string getAddress();
    std::string getPort();

private:
    QIntValidator m_portValidator;
    Ui::DGLConnectDialogClass m_ui;
};

#endif // DGLCONNECTDIALOG_H