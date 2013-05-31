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
#include "dglgui.h"

#include "dglshaderviewitem.h"
#include "dglglsleditor.h"
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>


//line numbering taken from  http://doc.qt.digia.com/4.6/widgets-codeeditor.html

class DGLLineNumberArea : public QWidget {
public:
    DGLLineNumberArea(DGLGLSLEditor *editor) : QWidget(editor),m_CodeEditor(editor) { }

    QSize sizeHint() const {
        return QSize(m_CodeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        m_CodeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    DGLGLSLEditor *m_CodeEditor;
};


DGLShaderViewItem::DGLShaderViewItem(dglnet::ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent),
        DGLRequestHandler(resManager->getRequestManager()), m_RequestManager(resManager->getRequestManager()), m_Name(name), m_EditState(EditState::S_ERRED_OR_UNAVAIL) {
    m_Ui.setupUi(this);
    m_GLSLEditor = new DGLGLSLEditor(this);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout->addWidget(m_Label);

    m_Ui.verticalLayout->insertWidget(0, m_GLSLEditor);
    m_Ui.verticalLayout->setStretch(0, 4);
    m_Ui.verticalLayout->setStretch(2, 1);

    m_Highlighter = boost::make_shared<DGLSyntaxHighlighterGLSL>(m_Ui.checkBox_Highlight->isChecked()?m_GLSLEditor->document():NULL);

    m_Listener = resManager->createListener(name, dglnet::DGLResource::ObjectTypeShader);
    m_Listener->setParent(this);

    editAction(EditAction::A_ERROR);

    CONNASSERT(connect(m_Ui.checkBox_Highlight, SIGNAL(toggled(bool)), this, SLOT(toggleHighlight(bool))));

    CONNASSERT(connect(m_Listener,SIGNAL(update(const dglnet::DGLResource&)),this,SLOT(update(const dglnet::DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));

    CONNASSERT(connect(m_GLSLEditor,SIGNAL(textChanged()),this,SLOT(editTextChanged())));


}

DGLShaderViewItem::~DGLShaderViewItem() {
    editAction(EditAction::A_ERROR);
}

void DGLShaderViewItem::editAction(EditAction action) {

    if (action == EditAction::A_ERROR) {
        //move to errored or uninitialized state
        //in this state only "Edit" is visible, but disabled
        m_EditState = EditState::S_ERRED_OR_UNAVAIL;
        m_GLSLEditor->setReadOnly(true);
        m_Ui.pushButtonEdit->setDisabled(true);
        m_Ui.pushButtonEdit->show();
        m_Ui.pushButtonResetEdits->hide();
        
    } else if (m_EditState == EditState::S_ERRED_OR_UNAVAIL && action == EditAction::A_NOERROR) {
        //move to DISABLED state, and allow user to click Edit button
        m_EditState = EditState::S_DISABLED;
        m_Ui.pushButtonEdit->setEnabled(true);

    } else if (m_EditState == EditState::S_DISABLED && action == EditAction::A_ENABLE) {
        
        //Start shader editing

        m_EditState = EditState::S_ENABLED;
        m_Ui.pushButtonEdit->hide();
        m_Ui.pushButtonResetEdits->show();
        m_GLSLEditor->setReadOnly(false);

        //do first edit - just to test if it is possible
        editTextChanged();

    } else if (m_EditState == EditState::S_ENABLED && action == EditAction::A_DISABLE) {

        //Stop shader editing

        m_EditState = EditState::S_DISABLED;
        m_Ui.pushButtonEdit->show();
        m_Ui.pushButtonResetEdits->hide();
        m_GLSLEditor->setReadOnly(true);

    } else if (m_EditState == EditState::S_ENABLED && action == EditAction::A_EDIT) {
        m_RequestManager->request(new dglnet::request::EditShaderSource(
            m_Name.m_Context, m_Name.m_Name, false, m_GLSLEditor->toPlainText().toStdString())
            , this);
    }
}

void DGLShaderViewItem::error(const std::string& message) {
    m_GLSLEditor->hide();
    m_Ui.groupBox1->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
    editAction(EditAction::A_ERROR);
}

void DGLShaderViewItem::saveShader() {
    QString fileName = QFileDialog::getSaveFileName(this, tr( "Save shader as..." ), QString(), tr( "Text files (*.txt)" ));
    QFile f(fileName);
    if  (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Write error"), tr("Cannot open file for writing."));
        return;
    }
    f.write(m_GLSLEditor->toPlainText().toLocal8Bit());
    f.close();
}

void DGLShaderViewItem::editStart() {
    editAction(EditAction::A_ENABLE);
}

void DGLShaderViewItem::editCancel() {
    
    m_RequestManager->request(new dglnet::request::EditShaderSource(
        m_Name.m_Context, m_Name.m_Name, true), this);

    editAction(EditAction::A_DISABLE);
}

void DGLShaderViewItem::editTextChanged() {
    editAction(EditAction::A_EDIT);
}

void DGLShaderViewItem::update(const dglnet::DGLResource& res) {
        
    m_GLSLEditor->show();
    m_Ui.groupBox1->show();
    m_Label->hide();
    const dglnet::resource::DGLResourceShader* resource = dynamic_cast<const dglnet::resource::DGLResourceShader*>(&res);

    m_Ui.textEditLinker->setText(QString::fromStdString(resource->m_CompileStatus.first));

    QString newSource = QString::fromStdString(resource->m_Source);
    if (newSource != m_GLSLEditor->toPlainText()) {
        //take off shader editing for a moment
        EditState lastEditState = EditState::S_ERRED_OR_UNAVAIL;
        std::swap(m_EditState, lastEditState);

        m_GLSLEditor->clear();
        m_GLSLEditor->appendPlainText(newSource);

        std::swap(m_EditState, lastEditState);
    }

    if (resource->m_ShaderObjDeleted) {
        m_Ui.shaderStatus->setText(tr("Shader object already deleted. Shown cached source."));
        editAction(EditAction::A_ERROR);
    } else {
        editAction(EditAction::A_NOERROR);
        if (!resource->m_CompileStatus.second) {
            m_Ui.shaderStatus->setText(tr("Compile status: failed"));
        } else {
            m_Ui.shaderStatus->setText(tr("Compile status: success"));
        }
    }    
}

void DGLShaderViewItem::toggleHighlight(bool enabled) {
    m_Highlighter->setDocument(enabled ? m_GLSLEditor->document() : NULL );
}

void DGLShaderViewItem::onRequestFinished(const dglnet::message::RequestReply* reply) {
    std::string replyStr;
    if (!reply->isOk(replyStr)) {
        QMessageBox::critical(this, "Cannot edit shader", QString::fromStdString(replyStr));
        editAction(EditAction::A_DISABLE);
    }
    m_Listener->fire();
}

    
DGLShaderView::DGLShaderView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shaders", "DGLShaderView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showShader(uint, uint, uint)), this, SLOT(showShader(uint, uint, uint))));
}

void DGLShaderView::showShader(uint ctx, uint name, uint target) {
    ensureTabDisplayed(ctx, name, target);
}

DGLTabbedViewItem* DGLShaderView::createTab(const dglnet::ContextObjectName& id) {
    return new DGLShaderViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLShaderView::getTabName(uint id, uint target) {
    return QString::fromStdString(GetShaderStageName(target)) + QString(" Shader ") + QString::number(id);
}