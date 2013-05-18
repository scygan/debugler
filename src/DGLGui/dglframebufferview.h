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


#ifndef DGLFRAMEBUFFERVIEW_H
#define DGLFRAMEBUFFERVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dglframebufferviewitem.h"

class DGLFramebufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFramebufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showFramebuffer(uint ctx, uint bufferEnum);

private:
        virtual DGLTabbedViewItem* createTab(const ContextObjectName& id);
        virtual QString getTabName(uint id, uint target);
};

class DGLFramebufferViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFramebufferViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);

private: 
    Ui::DGLFramebufferViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    DGLResourceListener* m_Listener;
    boost::shared_ptr<DGLPixelRectangle> m_PixelRectangle;
};

#endif //DGLFRAMEBUFFERVIEW_H