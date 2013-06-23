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


#ifndef DGLBUFFERVIEW_H
#define DGLBUFFERVIEW_H

#include "dgltabbedview.h"
#include "QHexEdit/qhexedit.h"

#include <QLabel>

class DGLBufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLBufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showBuffer(uint ctx, uint name);

private:
        virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
        virtual QString getTabName(uint id, uint target);
};

class DGLBufferViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLBufferViewItem(dglnet::ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const dglnet::DGLResource& res);

private: 
    QHexEdit* m_Editor;
    QLabel* m_Label;
    QVBoxLayout* m_VerticalLayout;
    DGLResourceListener* m_Listener;
};

#endif //DGLBUFFERVIEW_H