#ifndef DGLSHADERVIEWITEM_H
#define DGLSHADERVIEWITEM_H

#include "ui_dglshaderview.h"
#include "srchiliteqt/lib/srchiliteqt/Qt4SyntaxHighlighter.h"

class DGLGLSLEditor;

class DGLShaderViewItem: public DGLTabbedViewItem {
public:
    DGLShaderViewItem(uint name, QWidget* parrent);
    void update(const dglnet::ShaderMessage& msg);
    virtual void requestUpdate(DglController* controller);
    Ui::DGLShaderViewItem m_Ui;
    DGLGLSLEditor* m_GLSLEditor;
    boost::shared_ptr<srchiliteqt::Qt4SyntaxHighlighter> m_Highlighter;
};

#endif