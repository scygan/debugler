#include "dglbufferview.h"
#include "dglgui.h"

DGLBufferViewItem::DGLBufferViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Editor = new QHexEdit(this);
    m_Label = new QLabel(this);
    m_VerticalLayout = new QVBoxLayout(this);
    m_VerticalLayout->addWidget(m_Editor);
    m_VerticalLayout->addWidget(m_Label);

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeBuffer);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLBufferViewItem::error(const std::string& message) {
    m_Editor->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
}

void DGLBufferViewItem::update(const DGLResource& res) {
    m_Editor->show();
    m_Label->hide();
    const DGLResourceBuffer* resource = dynamic_cast<const DGLResourceBuffer*>(&res);
    QByteArray array(&resource->m_Data[0], resource->m_Data.size());
    m_Editor->setData(array);
}

DGLBufferView::DGLBufferView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Vertex Buffers", "DGLBufferView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showBuffer(uint, uint)), this, SLOT(showBuffer(uint, uint))));
}

void DGLBufferView::showBuffer(uint ctx, uint name) {
    ensureTabDisplayed(ctx, name);
}

DGLTabbedViewItem* DGLBufferView::createTab(const ContextObjectName& id) {
    return new DGLBufferViewItem(id, m_ResourceManager, this);
}

QString DGLBufferView::getTabName(uint id, uint target) {
    return QString("Vertex Buffer ") + QString::number(id);
}