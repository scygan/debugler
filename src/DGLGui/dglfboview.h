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


#ifndef DGLFBOVIEW_H
#define DGLFBOVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dglfboviewitem.h"

class DGLFBOViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFBOViewItem(dglnet::ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const dglnet::DGLResource&);
    void showAttachment(int id);

private: 
    Ui_DGLFBOViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    std::vector<dglnet::resource::DGLResourceFBO::FBOAttachment> m_Attachments;
    bool m_Error; 
    DGLResourceListener* m_Listener;
};



class DGLFBOView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFBOView(QWidget* parrent, DglController* controller);

    public slots:
        void showFBO(uint ctx, uint name);

private:
        virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
        virtual QString getTabName(uint id, uint target);
};

#endif //DGLFRAMEBUFFERVIEW_H