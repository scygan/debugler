#ifndef DGLSHADERVIEW_H
#define DGLSHADERVIEW_H

#include "dgltabbedview.h"
#include <QPlainTextEdit>

class DGLShaderView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLShaderView(QWidget* parrent, DglController* controller);

    public slots:
        void showShader(uint, uint);
        void gotShader(uint, const dglnet::ShaderMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);

};

class DGLGLSLEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    DGLGLSLEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

#endif //DGLSHADERVIEW_H