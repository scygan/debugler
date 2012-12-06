#include "dgltextureview.h"
#include "dglgui.h"

DGLTextureViewItem::DGLTextureViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    
    m_PixelRectangleScene = new DGLPixelRectangleScene();
    m_PixelRectangleView = boost::make_shared<DGLPixelRectangleView>(this, m_PixelRectangleScene);
    m_PixelRectangleView->setMinimumSize(QSize(400, 320));
    m_Ui.verticalLayout->insertWidget(0, m_PixelRectangleView.get());

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeTexture);
    m_Listener->setParent(this);

    m_Ui.horizontalSlider_LOD->setDisabled(true);

    CONNASSERT(connect(m_Ui.horizontalSlider_LOD,SIGNAL(sliderMoved(int)),this,SLOT(sliderMoved(int))));

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLTextureViewItem::error(const std::string& message) {
    m_PixelRectangleScene->setText(message);
    m_Ui.horizontalSlider_LOD->setDisabled(true);
}

void DGLTextureViewItem::update(const DGLResource& res) {
    const DGLResourceTexture* resource = dynamic_cast<const DGLResourceTexture*>(&res);
    
    m_Levels = resource->m_Levels;
    
    m_Ui.horizontalSlider_LOD->setRange(0, m_Levels.size() - 1);
    m_Ui.horizontalSlider_LOD->setValue(0);
    sliderMoved(0);
    m_Ui.horizontalSlider_LOD->setEnabled(true);
}

void DGLTextureViewItem::sliderMoved(int value) {
    if (value >= 0 && value < m_Levels.size()) {
        m_PixelRectangleScene->setPixelRectangle(&m_Levels[value]);
    } else {
        m_PixelRectangleScene->setText("Unknown LOD");
    }    
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