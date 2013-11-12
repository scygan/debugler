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

#include "dglglsleditor.h"

#include <QPainter>
#include <QTextBlock>

// line numbering taken from
// http://doc.qt.digia.com/4.6/widgets-codeeditor.html

class DGLLineNumberArea : public QWidget {
   public:
    DGLLineNumberArea(DGLGLSLEditor *editor)
            : QWidget(editor), m_CodeEditor(editor) {}

    QSize sizeHint() const {
        return QSize(m_CodeEditor->lineNumberAreaWidth(), 0);
    }

   protected:
    void paintEvent(QPaintEvent *_event) {
        m_CodeEditor->lineNumberAreaPaintEvent(_event);
    }

   private:
    DGLGLSLEditor *m_CodeEditor;
};

DGLGLSLEditor::DGLGLSLEditor(QWidget *_parent) : QPlainTextEdit(_parent) {
    lineNumberArea = new DGLLineNumberArea(this);

    CONNASSERT(this, SIGNAL(blockCountChanged(int)), this,
               SLOT(updateLineNumberAreaWidth(int)));
    CONNASSERT(this, SIGNAL(updateRequest(QRect, int)), this,
               SLOT(updateLineNumberArea(QRect, int)));
    CONNASSERT(this, SIGNAL(cursorPositionChanged()), this,
               SLOT(highlightCurrentLine()));

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

void DGLGLSLEditor::updateLineNumberArea(const QRect &_rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, _rect.y(), lineNumberArea->width(),
                               _rect.height());

    if (_rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void DGLGLSLEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(
            QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void DGLGLSLEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void DGLGLSLEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> newExtraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        newExtraSelections.append(selection);
    }

    setExtraSelections(newExtraSelections);
}

void DGLGLSLEditor::lineNumberAreaPaintEvent(QPaintEvent *_event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(_event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
            (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    while (block.isValid() && top <= _event->rect().bottom()) {
        if (block.isVisible() && bottom >= _event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(),
                             fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++blockNumber;
    }
}
