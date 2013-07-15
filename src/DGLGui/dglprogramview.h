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


#ifndef DGLPROGRAMVIEW_H
#define DGLPROGRAMVIEW_H

#include "dgltabbedview.h"
#include "ui_dglprogramview.h"
#include <QPlainTextEdit>

class DGLProgramView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLProgramView(QWidget* parrent, DglController* controller);

    public slots:
        void showProgram(opaque_id_t ctx, gl_t name);

private:
        virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
        virtual QString getTabName(gl_t id, gl_t target) override;

};

class DGLProgramViewItem: public DGLTabbedViewItem, public DGLRequestHandler {
    Q_OBJECT
public:
    DGLProgramViewItem(dglnet::ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const dglnet::DGLResource& res);
    void forceLink();

private:

    virtual void onRequestFinished(const dglnet::message::RequestReply* reply);

    Ui::DGLProgramViewItem m_Ui;
    QLabel* m_Label;
    DGLResourceListener* m_Listener;
    DGLRequestManager* m_RequestManager;
    DGLResourceManager* m_ResourceManager;
    dglnet::ContextObjectName m_Name;
};

#endif //DGLPROGRAMVIEW_H