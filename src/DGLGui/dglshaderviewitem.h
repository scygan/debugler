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

class DGLShaderViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLShaderViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void update(const DGLResource& res);
    void error(const std::string&);
    void toggleHighlight(bool);
private:
    Ui::DGLShaderViewItem m_Ui;
    QLabel* m_Label;
    DGLGLSLEditor* m_GLSLEditor;
    boost::shared_ptr<DGLSyntaxHighlighterGLSL> m_Highlighter;
    DGLResourceListener* m_Listener;
};

#endif