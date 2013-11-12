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

#include "dglbufferview.h"

DGLBufferViewItem::DGLBufferViewItem(dglnet::ContextObjectName name,
                                     DGLResourceManager* resManager,
                                     QWidget* parrent)
        : DGLTabbedViewItem(name, parrent) {
    m_Editor = new QHexEdit(this);
    m_Label = new QLabel(this);
    m_VerticalLayout = new QVBoxLayout(this);
    m_VerticalLayout->addWidget(m_Editor);
    m_VerticalLayout->addWidget(m_Label);

    m_Listener = resManager->createListener(
            name, dglnet::DGLResource::ObjectType::Buffer);
    m_Listener->setParent(this);

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));
}

void DGLBufferViewItem::error(const std::string& message) {
    m_Editor->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
}

void DGLBufferViewItem::update(const dglnet::DGLResource& res) {
    m_Editor->show();
    m_Label->hide();
    const dglnet::resource::DGLResourceBuffer* resource =
            dynamic_cast<const dglnet::resource::DGLResourceBuffer*>(&res);
    QByteArray array(&resource->m_Data[0], resource->m_Data.size());
    m_Editor->setData(array);
}

DGLBufferView::DGLBufferView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Vertex Buffers", "DGLBufferView");

    // inbound
    CONNASSERT(controller->getViewRouter(),
               SIGNAL(showBuffer(opaque_id_t, gl_t)), this,
               SLOT(showBuffer(opaque_id_t, gl_t)));
}

void DGLBufferView::showBuffer(opaque_id_t ctx, gl_t name) {
    ensureTabDisplayed(ctx, name);
}

DGLTabbedViewItem* DGLBufferView::createTab(
        const dglnet::ContextObjectName& id) {
    return new DGLBufferViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLBufferView::getTabName(gl_t id, gl_t /*target*/) {
    return QString("Vertex Buffer ") + QString::number(id);
}
