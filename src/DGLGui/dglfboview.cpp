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

#include "dglfboview.h"

#include "ui_dglfboviewitem.h"

DGLFBOViewItem::DGLFBOViewItem(dglnet::ContextObjectName name,
                               DGLResourceManager* resManager, QWidget* parrent)
        : DGLTabbedViewItem(name, parrent), m_Error(false) {
    m_Ui.setupUi(this);
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_Ui.m_pixelRectangleView->setScene(m_PixelRectangleScene);

    m_Listener =
        resManager->createListener(name, dglnet::DGLResource::ObjectType::FBO);
    m_Listener->setParent(this);

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));
}

void DGLFBOViewItem::error(const std::string& message) {
    m_Ui.m_AttListWidget->clear();
    m_PixelRectangleScene->setText(message);
    m_Error = true;
    m_Ui.m_pixelRectangleView->updateFormatSizeInfo(NULL);
}

void DGLFBOViewItem::update(const dglnet::DGLResource& res) {

    const dglnet::resource::DGLResourceFBO* resource =
        dynamic_cast<const dglnet::resource::DGLResourceFBO*>(&res);

    m_Ui.m_AttListWidget->clear();
    m_Error = false;
    m_Attachments = resource->m_Attachments;
    for (size_t i = 0; i < resource->m_Attachments.size(); i++) {
        m_Ui.m_AttListWidget->addItem(QString::fromStdString(
            GetGLEnumName(resource->m_Attachments[i].m_Id)));
    }
    showAttachment(0);
}

void DGLFBOViewItem::showAttachment(int id) {
    if (m_Error || static_cast<size_t>(id) >= m_Attachments.size() || id < 0)
        return;

    std::string errorMsg;

    if (!m_Attachments[id].isOk(errorMsg)) {
        m_PixelRectangleScene->setText(errorMsg);
    } else {
        m_PixelRectangleScene->setPixelRectangle(
            *m_Attachments[id].m_PixelRectangle.get());
        m_Ui.m_pixelRectangleView->updateFormatSizeInfo(
            (m_Attachments[id].m_PixelRectangle.get()));
    }
}

DGLFBOView::DGLFBOView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Framebuffer Objects", "DGLFBOView");

    // inbound
    CONNASSERT(controller->getViewRouter(), SIGNAL(showFBO(opaque_id_t, gl_t)),
               this, SLOT(showFBO(opaque_id_t, gl_t)));
}

void DGLFBOView::showFBO(opaque_id_t ctx, gl_t bufferEnum) {
    ensureTabDisplayed(ctx, bufferEnum);
}

DGLTabbedViewItem* DGLFBOView::createTab(const dglnet::ContextObjectName& id) {
    return new DGLFBOViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLFBOView::getTabName(gl_t id, gl_t /*target*/) {
    return QString("FBO ") + QString::number(id);
}
