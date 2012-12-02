#include "dglshaderview.h"
#include "dglgui.h"

#include "dglshaderviewitem.h"
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


DGLGLSLEditor::DGLGLSLEditor(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new DGLLineNumberArea(this);

    CONNASSERT(connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int))));
    CONNASSERT(connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int))));
    CONNASSERT(connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine())));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int DGLGLSLEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void DGLGLSLEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void DGLGLSLEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void DGLGLSLEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void DGLGLSLEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void DGLGLSLEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}


DGLShaderViewItem::DGLShaderViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_GLSLEditor = new DGLGLSLEditor(this);

    //we may modify this in far future :)
    m_GLSLEditor->setReadOnly(true);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout->addWidget(m_Label);

    m_Ui.verticalLayout->insertWidget(0, m_GLSLEditor);
    m_Ui.verticalLayout->setStretch(0, 4);
    m_Ui.verticalLayout->setStretch(1, 1);

    m_Highlighter = boost::make_shared<srchiliteqt::Qt4SyntaxHighlighter>(m_GLSLEditor->document());
    m_Highlighter->init("glsl.lang");

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeShader);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));

}

void DGLShaderViewItem::error(const std::string& message) {
    m_GLSLEditor->hide();
    m_Ui.groupBox1->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
}

void DGLShaderViewItem::update(const DGLResource& res) {
    m_GLSLEditor->show();
    m_Ui.groupBox1->show();
    m_Label->hide();
    const DGLResourceShader* resource = dynamic_cast<const DGLResourceShader*>(&res);

    m_Ui.textEditLinker->setText(QString::fromStdString(resource->m_CompileStatus.first));
    m_GLSLEditor->clear();
    for (size_t i = 0; i < resource->m_Sources.size(); i++) {
        m_GLSLEditor->appendPlainText(QString::fromStdString(resource->m_Sources[i]));
    }
    if (!resource->m_CompileStatus.second) {
        m_Ui.labelLinkStatus->setText(tr("Compile status: failed"));
    } else {
        m_Ui.labelLinkStatus->setText(tr("Compile status: success"));
    }
}
    
DGLShaderView::DGLShaderView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shaders", "DGLShaderView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showShader(uint, uint)), this, SLOT(showShader(uint, uint))));
}

void DGLShaderView::showShader(uint name, uint target) {
    ensureTabDisplayed(name, target);
}

DGLTabbedViewItem* DGLShaderView::createTab(uint id) {
    return new DGLShaderViewItem(id, m_ResourceManager, this);
}

QString DGLShaderView::getTabName(uint id, uint target) {
    return QString(GetShaderStageName(target)) + QString(" Shader ") + QString::number(id);
}