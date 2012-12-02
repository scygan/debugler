#ifndef DGLSHADERVIEWITEM_H
#define DGLSHADERVIEWITEM_H

#include "dgltabbedview.h"
#include "ui_dglshaderview.h"
#include "srchiliteqt/lib/srchiliteqt/Qt4SyntaxHighlighter.h"

class DGLGLSLEditor;

class DGLShaderViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLShaderViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void update(const DGLResource& res);
    void error(const std::string&);
private:
    Ui::DGLShaderViewItem m_Ui;
    QLabel* m_Label;
    DGLGLSLEditor* m_GLSLEditor;
    boost::shared_ptr<srchiliteqt::Qt4SyntaxHighlighter> m_Highlighter;
    DGLResourceListener* m_Listener;
};

#endif