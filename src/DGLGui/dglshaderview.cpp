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

#include "dglshaderview.h"

#include "dglshaderviewitem.h"
#include "dglglsleditor.h"
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

#include <DGLNet/protocol/request.h>
#include <DGLNet/protocol/message.h>
#include <DGLNet/protocol/resource.h>

// line numbering taken from
// http://doc.qt.digia.com/4.6/widgets-codeeditor.html

class DGLLineNumberArea : public QWidget {
   public:
    DGLLineNumberArea(DGLGLSLEditor* editor)
            : QWidget(editor), m_CodeEditor(editor) {}

    QSize sizeHint() const {
        return QSize(m_CodeEditor->lineNumberAreaWidth(), 0);
    }

   protected:
    void paintEvent(QPaintEvent* _event) {
        m_CodeEditor->lineNumberAreaPaintEvent(_event);
    }

   private:
    DGLGLSLEditor* m_CodeEditor;
};

DGLShaderViewItem::DGLShaderViewItem(dglnet::ContextObjectName name,
                                     DGLResourceManager* resManager,
                                     QWidget* parrent)
        : DGLTabbedViewItem(name, parrent),
          m_EditRequestHandler(this, resManager->getRequestManager()),
          m_ResetRequestHandler(this, resManager->getRequestManager()),
          m_RequestManager(resManager->getRequestManager()),
          m_Name(name) {
    m_Ui.setupUi(this);

    m_GLSLEditor = new DGLGLSLEditor(this);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout->addWidget(m_Label);

    m_Ui.verticalLayout->insertWidget(0, m_GLSLEditor);
    m_Ui.verticalLayout->setStretch(0, 4);
    m_Ui.verticalLayout->setStretch(2, 1);

    m_Listener = resManager->createListener(
            name, dglnet::message::ObjectType::Shader);
    m_Listener->setParent(this);

    // cannot edit shader now: no shader to edit
    setState(EditState::S_UNAVAILABLE);

    CONNASSERT(m_Ui.checkBox_Highlight, SIGNAL(toggled(bool)), this,
               SLOT(toggleHighlight(bool)));

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));

    CONNASSERT(m_GLSLEditor, SIGNAL(textChanged()), this,
               SLOT(editTextChanged()));
}

DGLShaderViewItem::~DGLShaderViewItem() {
    // notify: no shader to edit during destruction
    editAction(EditAction::A_NOTIFY_ERROR);
}

void DGLShaderViewItem::editAction(EditAction action) {

    if (action == EditAction::A_NOTIFY_ERROR) {
        if (m_EditState == EditState::S_EDITING) {
            setState(EditState::S_PAUSE);
        } else if (m_EditState != EditState::S_PAUSE) {
            setState(EditState::S_UNAVAILABLE);
        }
    } else if (action == EditAction::A_NOTIFY_NOERROR) {
        if (m_EditState == EditState::S_PAUSE) {
            setState(EditState::S_EDITING);
        } else if (m_EditState == EditState::S_UNAVAILABLE) {
            setState(EditState::S_NOT_EDITING);
        }
    } else if (m_EditState == EditState::S_NOT_EDITING &&
               action == EditAction::A_ENABLE) {

        // Start shader editing

        setState(EditState::S_EDITING);

        // do first edit - just to test if it is possible
        editTextChanged();

    } else if (m_EditState == EditState::S_EDITING &&
               action == EditAction::A_DISABLE) {

        // Stop shader editing
        setState(EditState::S_NOT_EDITING);

    } else if (m_EditState == EditState::S_EDITING &&
               action == EditAction::A_EDIT) {
        m_RequestManager->request(
                new dglnet::request::EditShaderSource(
                        m_Name.m_Context, m_Name.m_Name, false,
                        m_GLSLEditor->toPlainText().toStdString()),
                &m_EditRequestHandler);
    }
}

void DGLShaderViewItem::setState(EditState editState) {
    switch (editState) {
        case EditState::S_UNAVAILABLE:
            m_GLSLEditor->setReadOnly(true);
            m_Ui.pushButtonEdit->setDisabled(true);
            m_Ui.pushButtonEdit->show();
            m_Ui.pushButtonResetEdits->hide();
            break;
        case EditState::S_NOT_EDITING:
            m_GLSLEditor->setReadOnly(true);
            m_Ui.pushButtonEdit->show();
            m_Ui.pushButtonEdit->setEnabled(true);
            m_Ui.pushButtonResetEdits->hide();
            break;
        case EditState::S_PAUSE:
            m_GLSLEditor->setReadOnly(true);
            m_Ui.pushButtonEdit->hide();
            m_Ui.pushButtonResetEdits->show();
            m_Ui.pushButtonResetEdits->setEnabled(false);
            break;
        case EditState::S_EDITING:
            m_GLSLEditor->setReadOnly(false);
            m_Ui.pushButtonEdit->hide();
            m_Ui.pushButtonResetEdits->show();
            m_Ui.pushButtonResetEdits->setEnabled(true);
            break;
        default:
            DGL_ASSERT(0);
    }
    m_EditState = editState;
}

void DGLShaderViewItem::error(const std::string& message) {
    m_GLSLEditor->hide();
    m_Ui.groupBox1->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
    // notify: no shader to edit due to error
    editAction(EditAction::A_NOTIFY_ERROR);
}

void DGLShaderViewItem::saveShader() {
    QString fileName = QFileDialog::getSaveFileName(
            this, tr("Save shader as..."), QString(), tr("Text files (*.txt)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Write error"),
                              tr("Cannot open file for writing."));
        return;
    }
    f.write(m_GLSLEditor->toPlainText().toLocal8Bit());
    f.close();
}

void DGLShaderViewItem::editStart() { editAction(EditAction::A_ENABLE); }

void DGLShaderViewItem::editReset() {

    m_RequestManager->request(new dglnet::request::EditShaderSource(
                                      m_Name.m_Context, m_Name.m_Name, true),
                              &m_ResetRequestHandler);
}

void DGLShaderViewItem::editTextChanged() { editAction(EditAction::A_EDIT); }

void DGLShaderViewItem::update(const dglnet::DGLResource& res) {

    m_GLSLEditor->show();
    m_Ui.groupBox1->show();
    m_Label->hide();
    const dglnet::resource::DGLResourceShader* resource =
            dynamic_cast<const dglnet::resource::DGLResourceShader*>(&res);

    if (!m_Highlighter) {
        m_Highlighter = std::make_shared<DGLSyntaxHighlighterGLSL>(
                resource->m_IsESSLDefault, m_Ui.checkBox_Highlight->isChecked()
                                                   ? m_GLSLEditor->document()
                                                   : NULL);
    }

    m_Ui.textEditLinker->setText(
            QString::fromStdString(resource->m_CompileStatus.first));

    QString newSource = QString::fromStdString(resource->m_Source);
    if (newSource != m_GLSLEditor->toPlainText()) {
        // take off shader editing for a moment
        EditState lastEditState = EditState::S_PAUSE;
        std::swap(m_EditState, lastEditState);

        m_GLSLEditor->clear();
        m_GLSLEditor->appendPlainText(newSource);

        std::swap(m_EditState, lastEditState);
    }

     // have shader to edit
    editAction(EditAction::A_NOTIFY_NOERROR);
    if (!resource->m_CompileStatus.second) {
        m_Ui.shaderStatus->setText(tr("Compile status: failed"));
    } else {
        m_Ui.shaderStatus->setText(tr("Compile status: success"));
    }
}

void DGLShaderViewItem::toggleHighlight(bool enabled) {
    m_Highlighter->setDocument(enabled ? m_GLSLEditor->document() : NULL);
}

DGLShaderViewItem::EditRequestHandler::EditRequestHandler(
        DGLShaderViewItem* parrent, DGLRequestManager* manager)
        : DGLRequestHandler(manager), m_Parrent(parrent) {}

void DGLShaderViewItem::EditRequestHandler::onRequestFinished(
        const dglnet::message::utils::ReplyBase*) {

    m_Parrent->m_Listener->fire();
}

void DGLShaderViewItem::EditRequestHandler::onRequestFailed(
    const std::string& reply) {

    QMessageBox::critical(m_Parrent, "Cannot edit shader",
        QString::fromStdString(reply));
    m_Parrent->editAction(EditAction::A_NOTIFY_ERROR);
}

DGLShaderViewItem::ResetRequestHandler::ResetRequestHandler(
        DGLShaderViewItem* parrent, DGLRequestManager* manager)
        : DGLRequestHandler(manager), m_Parrent(parrent) {}

void DGLShaderViewItem::ResetRequestHandler::onRequestFinished(
        const dglnet::message::utils::ReplyBase*) {
    
    m_Parrent->editAction(EditAction::A_DISABLE);
    m_Parrent->m_Listener->fire();
}

void DGLShaderViewItem::ResetRequestHandler::onRequestFailed(
    const std::string& reply) {
       
    QMessageBox::critical(m_Parrent, "Cannot reset shader edits",
        QString::fromStdString(reply));
        
    m_Parrent->m_Listener->fire();
}

DGLShaderView::DGLShaderView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Shaders", "DGLShaderView");

    // inbound
    CONNASSERT(controller->getViewRouter(),
               SIGNAL(showShader(opaque_id_t, gl_t, gl_t)), this,
               SLOT(showShader(opaque_id_t, gl_t, gl_t)));
}

void DGLShaderView::showShader(opaque_id_t ctx, gl_t name, gl_t target) {
    ensureTabDisplayed(ctx, name, target);
}

DGLTabbedViewItem* DGLShaderView::createTab(
        const dglnet::ContextObjectName& id) {
    return new DGLShaderViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLShaderView::getTabName(gl_t id, gl_t target) {
    return QString::fromStdString(GetShaderStageName(target)) +
           QString(" Shader ") + QString::number(id);
}
