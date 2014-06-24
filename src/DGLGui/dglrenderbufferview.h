/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#ifndef DGLRENDERBUFFERVIEW_H
#define DGLRENDERBUFFERVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dglrenderbufferviewitem.h"

class DGLRenderbufferViewItem : public DGLTabbedViewItem {
    Q_OBJECT
   public:
    DGLRenderbufferViewItem(dglnet::ContextObjectName name,
                   DGLResourceManager* resManager, QWidget* parrent);

   private
slots:
    void error(const std::string& message);
    void update(const dglnet::DGLResource&);

   private:
    Ui_DGLRenderbufferViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;

    //this shared_pointer is from boost::
    ///because it is just copied from DGLNet resource
    boost::shared_ptr<const dglnet::resource::DGLPixelRectangle> m_PixelRectangle;

    DGLResourceListener* m_Listener;
};

class DGLRenderbufferView : public DGLTabbedView {
    Q_OBJECT

   public:
    DGLRenderbufferView(QWidget* parrent, DglController* controller);

   public
slots:
    void showRenderbuffer(opaque_id_t ctx, gl_t name);

   private:
    virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id);
    virtual QString getTabName(gl_t id, gl_t target) override;
    virtual QString getTabIcon() override;
};

#endif    // DGLFRAMEBUFFERVIEW_H