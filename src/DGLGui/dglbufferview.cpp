#include "dglbufferview.h"
#include "dglgui.h"

DGLBufferViewItem::DGLBufferViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    editor = new QHexEdit(this);
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(editor);

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeBuffer);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLBufferViewItem::error(const std::string& message) {
    //TODO
}

void DGLBufferViewItem::update(const DGLResource& res) {
    const DGLResourceBuffer* resource = dynamic_cast<const DGLResourceBuffer*>(&res);
    QByteArray array(&resource->m_Data[0], resource->m_Data.size());
    editor->setData(array);
}

DGLBufferView::DGLBufferView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Vertex Buffers", "DGLBufferView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showBuffer(uint)), this, SLOT(showBuffer(uint))));
}

void DGLBufferView::showBuffer(uint name) {
    ensureTabDisplayed(name);
}

DGLTabbedViewItem* DGLBufferView::createTab(uint id) {
    return new DGLBufferViewItem(id, m_ResourceManager, this);
}

QString DGLBufferView::getTabName(uint id, uint target) {
    return QString("Vertex Buffer ") + QString::number(id);
}