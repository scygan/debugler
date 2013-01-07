#include "dglframebufferview.h"
#include "dglgui.h"



DGLFramebufferViewItem::DGLFramebufferViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_PixelRectangleView = boost::make_shared<DGLPixelRectangleView>(this, m_PixelRectangleScene);
    m_PixelRectangleView->setMinimumSize(QSize(400, 320));
    m_Ui.verticalLayout->addWidget(m_PixelRectangleView.get());    
    

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeFramebuffer);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLFramebufferViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
    m_PixelRectangleView->updateFormatSizeInfo(NULL);
}

void DGLFramebufferViewItem::update(const DGLResource& res) {
    const DGLResourceFramebuffer* resource = dynamic_cast<const DGLResourceFramebuffer*>(&res);
    m_PixelRectangleScene->setPixelRectangle(&resource->m_PixelRectangle);
    m_PixelRectangleView->updateFormatSizeInfo(&resource->m_PixelRectangle);
}

DGLFramebufferView::DGLFramebufferView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Frame Buffers", "DGLFramebufferView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showFramebuffer(uint)), this, SLOT(showFramebuffer(uint))));
}

void DGLFramebufferView::showFramebuffer(uint bufferEnum) {
    ensureTabDisplayed(bufferEnum);
}

DGLTabbedViewItem* DGLFramebufferView::createTab(uint id) {
    return new DGLFramebufferViewItem(id, m_ResourceManager, this);
}

QString DGLFramebufferView::getTabName(uint id, uint target) {
    switch (id) {
        case GL_FRONT_RIGHT:
            return "Front right buffer"; 
        case GL_BACK_RIGHT:
            return "Back right buffer";
        case GL_FRONT:
            return "Front buffer";
        case GL_BACK:
            return "Back buffer";
    }
    return QString("Frame Buffer ") + QString::number(id);
}