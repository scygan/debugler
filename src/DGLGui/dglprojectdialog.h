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

#ifndef DGLPROJECTDIALOG_H
#define DGLPROJECTDIALOG_H


#include "dglqtgui.h"
#include "ui_dglprojectdialog.h"

#include "dglproject.h"
#include <memory>
#include <vector>

class DGLProjectDialog : public QDialog {
    Q_OBJECT

public:
    DGLProjectDialog();
    ~DGLProjectDialog();

    std::shared_ptr<DGLProject> getProject();

public slots:
    void projectTypeSelected(int);

private:

    void addProjectFactory(std::shared_ptr<DGLProjectFactory> factory);

    std::vector<std::shared_ptr<DGLProjectFactory> > m_Factories;
    Ui::DGLProjectDialogClass m_ui;
};

#endif