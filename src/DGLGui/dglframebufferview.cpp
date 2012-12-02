#include "dglframebufferview.h"
#include "dglgui.h"



DGLFramebufferViewItem::DGLFramebufferViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
    m_Ui.setupUi(this);
    m_Scene = boost::make_shared<QGraphicsScene>(this);
    m_Scene->setSceneRect(0, 0, 400, 320);
    m_Ui.graphicsView->setScene(m_Scene.get());
    m_Ui.graphicsView->show();

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeFramebuffer);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLFramebufferViewItem::error(const std::string& message) {
    m_Scene->clear();
    m_Scene->addText(message.c_str());
}

void DGLFramebufferViewItem::update(const DGLResource& res) {
    const DGLResourceFramebuffer* resource = dynamic_cast<const DGLResourceFramebuffer*>(&res);
    m_Scene->clear();
    m_PixelData = std::vector<uchar>(resource->m_Pixels.begin(), resource->m_Pixels.end());

    uint realHeight = m_PixelData.size() / resource->m_Channels / resource->m_Width;

    assert(realHeight == resource->m_Height);

    QImage::Format format = QImage::Format_Invalid;            
    switch (resource->m_Channels) {
    case 4:
        format = QImage::Format_ARGB32;
        break;
    case 3:
        format = QImage::Format_RGB888;
        break;
    default:
        assert(0);
    }
    m_Scene->addPixmap(QPixmap::fromImage(QImage(&m_PixelData[0], resource->m_Width, realHeight, format)));
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