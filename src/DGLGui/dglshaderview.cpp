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


DGLShaderViewItem::DGLShaderViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_GLSLEditor = new DGLGLSLEditor(this);

    //we may modify this in far future :)
    m_GLSLEditor->setReadOnly(true);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout->addWidget(m_Label);

    m_Ui.verticalLayout->insertWidget(0, m_GLSLEditor);
    m_Ui.verticalLayout->setStretch(0, 4);
    m_Ui.verticalLayout->setStretch(2, 1);

    m_Highlighter = boost::make_shared<DGLSyntaxHighlighterGLSL>(m_Ui.checkBox_Highlight->isChecked()?m_GLSLEditor->document():NULL);

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeShader);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Ui.checkBox_Highlight, SIGNAL(toggled(bool)), this, SLOT(toggleHighlight(bool))));

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));

}

void DGLShaderViewItem::error(const std::string& message) {
    m_GLSLEditor->hide();
    m_Ui.groupBox1->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
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

void DGLShaderViewItem::update(const DGLResource& res) {
    m_GLSLEditor->show();
    m_Ui.groupBox1->show();
    m_Label->hide();
    const DGLResourceShader* resource = dynamic_cast<const DGLResourceShader*>(&res);

    m_Ui.textEditLinker->setText(QString::fromStdString(resource->m_CompileStatus.first));
    m_GLSLEditor->clear();
    m_GLSLEditor->appendPlainText(QString::fromStdString(resource->m_Source));
    if (resource->m_ShaderObjDeleted) {
        m_Ui.shaderStatus->setText(tr("Shader object already deleted. Shown cached source."));
    } else {
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

    
DGLShaderView::DGLShaderView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shaders", "DGLShaderView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showShader(uint, uint, uint)), this, SLOT(showShader(uint, uint, uint))));
}

void DGLShaderView::showShader(uint ctx, uint name, uint target) {
    ensureTabDisplayed(ctx, name, target);
}

DGLTabbedViewItem* DGLShaderView::createTab(const ContextObjectName& id) {
    return new DGLShaderViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLShaderView::getTabName(uint id, uint target) {
    return QString::fromStdString(GetShaderStageName(target)) + QString(" Shader ") + QString::number(id);
}