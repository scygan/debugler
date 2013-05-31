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


#ifndef DGLSHADERVIEWITEM_H
#define DGLSHADERVIEWITEM_H

#include "dgltabbedview.h"
#include "ui_dglshaderview.h"
#include "dglsyntaxhighlight.h"

class DGLGLSLEditor;

class DGLShaderViewItem: public DGLTabbedViewItem, public DGLRequestHandler {
    Q_OBJECT
public:
    DGLShaderViewItem(dglnet::ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);
    ~DGLShaderViewItem();

public slots:
    void saveShader();
    void editStart();
    void editCancel();
    void editTextChanged();

private slots:
    void update(const dglnet::DGLResource& res);
    void error(const std::string&);
    void toggleHighlight(bool);
private:

    enum class EditState {
        S_ERRED_OR_UNAVAIL,
        S_DISABLED,
        S_ENABLED        
    };

    enum class EditAction {
        A_NOERROR,
        A_ERROR,
        A_DISABLE,        
        A_ENABLE,
        A_EDIT,
    };

    void editAction(EditAction);

    virtual void onRequestFinished(const dglnet::message::RequestReply* reply);

    Ui::DGLShaderViewItem m_Ui;
    QLabel* m_Label;
    DGLGLSLEditor* m_GLSLEditor;
    boost::shared_ptr<DGLSyntaxHighlighterGLSL> m_Highlighter;
    DGLResourceListener* m_Listener;
    DGLRequestManager* m_RequestManager;
    dglnet::ContextObjectName m_Name;
    EditState m_EditState;
};

#endif