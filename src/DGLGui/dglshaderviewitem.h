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

class DGLShaderViewItem : public DGLTabbedViewItem {
    Q_OBJECT
   public:
    DGLShaderViewItem(dglnet::ContextObjectName name,
                      DGLResourceManager* resManager, QWidget* parrent);
    ~DGLShaderViewItem();

   public
slots:
    void saveShader();
    void editStart();
    void editReset();
    void editTextChanged();

   private
slots:
    void update(const dglnet::DGLResource& res);
    void error(const std::string&);
    void toggleHighlight(bool);

   private:
    enum class EditState {
        STATE_EDITING_NOT_AVAIL,  // no shader to edit
        STATE_NOT_EDITING,        // can enter edit
        STATE_EDITING,            // editing now
        STATE_EDITING_PAUSED     // editing pause (no shader to edit arised while editing.
                                  // may resume edits later)
    };

    enum class EditAction {
        ACTION_ERROR,            // update error or no shader to edit
        ACTION_DISABLE_EDITING,  // disable shader editing (and reset to default source)
        ACTION_ENABLE_EDITING,   // enter shader editing
        ACTION_EDIT,             // edit shader
    };

    void editAction(EditAction);

    void setState(EditState);

    class EditRequestHandler : public DGLRequestHandler {
       public:
        EditRequestHandler(DGLShaderViewItem*, DGLRequestManager*);

       private:
           virtual void onRequestFinished(
               const dglnet::message::utils::ReplyBase* reply) override;
           virtual void onRequestFailed(
               const std::string& error) override;
        DGLShaderViewItem* m_Parrent;
    } m_EditRequestHandler;

    class ResetRequestHandler : public DGLRequestHandler {
       public:
        ResetRequestHandler(DGLShaderViewItem*, DGLRequestManager*);

       private:
        virtual void onRequestFinished(
                const dglnet::message::utils::ReplyBase* reply) override;
        virtual void onRequestFailed(
            const std::string& reply) override;
        DGLShaderViewItem* m_Parrent;
    } m_ResetRequestHandler;

    friend class EditRequestHandler;
    friend class ResetRequestHandler;

    Ui::DGLShaderViewItem m_Ui;
    QLabel* m_Label;
    DGLGLSLEditor* m_GLSLEditor;
    std::shared_ptr<DGLSyntaxHighlighterGLSL> m_Highlighter;
    DGLResourceListener* m_Listener;
    DGLRequestManager* m_RequestManager;
    dglnet::ContextObjectName m_Name;
    EditState m_EditState;
};

#endif