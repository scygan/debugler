#include "dgltextureview.h"
#include "dglgui.h"
#include <QGraphicsTextItem>


DGLTextureViewItem::DGLTextureViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_Scene = boost::make_shared<QGraphicsScene>(this);
    m_Scene->setSceneRect(0, 0, 400, 320);
    m_Ui.graphicsView->setScene(m_Scene.get());
    m_Ui.graphicsView->show();

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeTexture);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLTextureViewItem::error(const std::string& message) {
    m_Scene->clear();
    m_Scene->addText(message.c_str());
}

void DGLTextureViewItem::update(const DGLResource& res) {

    const DGLResourceTexture* resource = dynamic_cast<const DGLResourceTexture*>(&res);

    m_Scene->clear();
    m_PixelData = std::vector<uchar>(resource->m_Levels[0].m_Pixels.begin(), resource->m_Levels[0].m_Pixels.end());

    uint realHeight = m_PixelData.size() / resource->m_Levels[0].m_Channels / resource->m_Levels[0].m_Width;

    assert(realHeight == resource->m_Levels[0].m_Height);

    QImage::Format format = QImage::Format_Invalid;            
    switch (resource->m_Levels[0].m_Channels) {
    case 4:
        format = QImage::Format_ARGB32;
        break;
    case 3:
        format = QImage::Format_RGB888;
        break;
    default:
        assert(0);
    }
    m_Scene->addPixmap(QPixmap::fromImage(QImage(&m_PixelData[0], resource->m_Levels[0].m_Width, realHeight, format)));
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