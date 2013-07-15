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


#ifndef DGLSTATEVIEW_H
#define DGLSTATEVIEW_H

#include "dglcontroller.h"
#include "ui_dglstateview.h"

#include <QDockWidget>


class DGLStateView : public QDockWidget {
    Q_OBJECT

public:
    DGLStateView(QWidget* parrent, DglController* controller);

public slots:
    void setConnected(bool);
    void update(const dglnet::DGLResource&);
    void error(const std::string&);

private: 
    DGLResourceListener * m_Listener;
    DglController* m_Controller;
    Ui::DGLStateView* m_Ui;
};

#endif // DGLTREEVIEW_H