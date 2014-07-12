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

#include "dglrenderbufferview.h"

#include "ui_dglrenderbufferviewitem.h"

#include "DGLNet/protocol/shared_ptr_converter.h"

DGLRenderbufferViewItem::DGLRenderbufferViewItem(dglnet::ContextObjectName name,
                               DGLResourceManager* resManager, QWidget* parrent)
        : DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_Ui.m_pixelRectangleView->setScene(m_PixelRectangleScene);

    m_Listener = resManager->createListener(
            name, dglnet::message::ObjectType::Renderbuffer);
    m_Listener->setParent(this);

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));
}

void DGLRenderbufferViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
    m_Ui.m_pixelRectangleView->updateFormatSizeInfo(NULL, 0, 0);
}

void DGLRenderbufferViewItem::update(const dglnet::DGLResource& res) {

    const dglnet::resource::DGLResourceRenderbuffer* resource =
            dynamic_cast<const dglnet::resource::DGLResourceRenderbuffer*>(&res);


    m_PixelRectangle = resource->m_PixelRectangle;

    m_PixelRectangleScene->setPixelRectangle(
        *m_PixelRectangle.get());

    m_Ui.m_pixelRectangleView->updateFormatSizeInfo(
        m_PixelRectangle.get(),
        resource->m_Internalformat,
        resource->m_Samples);
}

DGLRenderbufferView::DGLRenderbufferView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Renderbuffer Objects", "DGLRenderbufferView");

    // inbound
    CONNASSERT(controller->getViewRouter(), SIGNAL(showRenderbuffer(opaque_id_t, gl_t)),
               this, SLOT(showRenderbuffer(opaque_id_t, gl_t)));
}

void DGLRenderbufferView::showRenderbuffer(opaque_id_t ctx, gl_t bufferEnum) {
    ensureTabDisplayed(ctx, bufferEnum);
}

DGLTabbedViewItem* DGLRenderbufferView::createTab(const dglnet::ContextObjectName& id) {
    return new DGLRenderbufferViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLRenderbufferView::getTabName(gl_t id, gl_t /*target*/) {
    return QString("Renderbuffer ") + QString::number(id);
}

QString DGLRenderbufferView::getTabIcon() {
    return DGL_RES_ICON_RENDERBUFFER_PATH;
}
