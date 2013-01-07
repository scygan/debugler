#include "dglfboview.h"
#include "dglgui.h"

#include "ui_dglfboviewitem.h"


DGLFBOViewItem::DGLFBOViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent),m_Error(false) {
    m_Ui.setupUi(this);
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_PixelRectangleView = boost::make_shared<DGLPixelRectangleView>(this, m_PixelRectangleScene);
    m_PixelRectangleView->setMinimumSize(QSize(400, 320));
    m_Ui.verticalLayout->addWidget(m_PixelRectangleView.get());    

    //internal
    CONNASSERT(connect(m_Ui.m_AttListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showAttachment(int))));

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeFBO);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));

}

void DGLFBOViewItem::error(const std::string& message) {
    m_Ui.m_AttListWidget->clear();
    m_PixelRectangleScene->setText(message);
    m_Error = true;
    m_PixelRectangleView->updateFormatSizeInfo(NULL);
}

void DGLFBOViewItem::update(const DGLResource& res) {

    const DGLResourceFBO* resource = dynamic_cast<const DGLResourceFBO*>(&res);
    
    m_Ui.m_AttListWidget->clear();
    m_Error = false;
    m_Attachments = resource->m_Attachments;
    for (int i = 0; i < resource->m_Attachments.size(); i++) {
        m_Ui.m_AttListWidget->addItem(QString::fromStdString(GetGLEnumName(resource->m_Attachments[i].m_Id)));
    }
    showAttachment(0);
}

void DGLFBOViewItem::showAttachment(int id) {
    if (m_Error || id >= m_Attachments.size() || id < 0) return;

    std::string errorMsg;

    if (!m_Attachments[id].isOk(errorMsg)) {
        m_PixelRectangleScene->setText(errorMsg);
    } else {
        m_PixelRectangleScene->setPixelRectangle(&m_Attachments[id].m_PixelRectangle);
        m_PixelRectangleView->updateFormatSizeInfo((&m_Attachments[id].m_PixelRectangle));
    }
}


DGLFBOView::DGLFBOView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Framebuffer Objects", "DGLFBOView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showFBO(uint)), this, SLOT(showFBO(uint))));
}

void DGLFBOView::showFBO(uint bufferEnum) {
    ensureTabDisplayed(bufferEnum);
}

DGLTabbedViewItem* DGLFBOView::createTab(uint id) {
    return new DGLFBOViewItem(id, m_ResourceManager, this);
}

QString DGLFBOView::getTabName(uint id, uint target) {
    return QString("FBO ") + QString::number(id);
}