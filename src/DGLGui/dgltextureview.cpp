#include "dgltextureview.h"
#include "dglgui.h"

DGLTextureViewItem::DGLTextureViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_PixelRectangleView = boost::make_shared<DGLPixelRectangleView>(this, m_PixelRectangleScene);
    m_PixelRectangleView->setMinimumSize(QSize(400, 320));
    m_Ui.verticalLayout->addWidget(m_PixelRectangleView.get());

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeTexture);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLTextureViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
}

void DGLTextureViewItem::update(const DGLResource& res) {
    const DGLResourceTexture* resource = dynamic_cast<const DGLResourceTexture*>(&res);
    m_PixelRectangleScene->setPixelRectangle(&resource->m_Levels[0]);
}


DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Textures", "DGLTextureView");
   	
    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showTexture(uint)), this, SLOT(showTexture(uint))));
        
}

void DGLTextureView::showTexture(uint name) {
    ensureTabDisplayed(name);
}

DGLTabbedViewItem* DGLTextureView::createTab(uint id) {
    return new DGLTextureViewItem(id, m_ResourceManager, this);
}

QString DGLTextureView::getTabName(uint id, uint target) {
    return QString("Texture ") + QString::number(id);
}