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


#ifndef DGLRUNDIALOG_H
#define DGLRUNDIALOG_H

#include <QMainWindow>

#include "ui_dglrundialog.h"

class DGLRunDialog : public QDialog {
    Q_OBJECT

public:
    DGLRunDialog();
    ~DGLRunDialog();
    std::string getExecutable();
    std::string getCommandLineArgs();
    std::string getPath();
    bool getModeEGL();

private slots:
    void updatePath();
    void browseExecutable();
    void browseDirectory();

private:
    QIntValidator m_portValidator;
    Ui::DGLRunDialogClass m_ui;
};

#endif // DGLRUNDIALOG_H