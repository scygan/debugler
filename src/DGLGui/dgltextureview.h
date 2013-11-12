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

#ifndef DGLTEXTUREVIEW_H
#define DGLTEXTUREVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dgltextureviewitem.h"

class DGLTextureView : public DGLTabbedView {
    Q_OBJECT

   public:
    DGLTextureView(QWidget* parrent, DglController* controller);

   public
slots:
    void showTexture(opaque_id_t ctx, gl_t name);

   private:
    virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
    virtual QString getTabName(gl_t id, gl_t target) override;
};

class DGLTextureViewItem : public DGLTabbedViewItem {
    Q_OBJECT
   public:
    DGLTextureViewItem(dglnet::ContextObjectName name,
                       DGLResourceManager* resManager, QWidget* parrent);

   private
slots:
    void error(const std::string& message);
    void update(const dglnet::DGLResource& res);
    void levelSliderMoved(int value);
    void faceComboChanged(int value);

   private:
    void internalUpdate();

    Ui::DGLTextureViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    std::vector<std::vector<boost::shared_ptr<
            dglnet::resource::DGLPixelRectangle> > > m_FacesLevels;

    DGLResourceListener* m_Listener;

    uint m_CurrentLevel, m_CurrentFace;
};

#endif    // DGLTEXTUREVIEW_H
