#include "dgltextureview.h"
#include "dglgui.h"

#include "ui_dgltextureviewitem.h"

#include <QGraphicsTextItem>

class DGLTextureViewItem: public DGLTabbedViewItem {
public:
    DGLTextureViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
        m_Ui.setupUi(this);
        m_Scene = boost::make_shared<QGraphicsScene>(this);
        m_Scene->setSceneRect(0, 0, 400, 320);
        m_Ui.graphicsView->setScene(m_Scene.get());
        m_Ui.graphicsView->show();
        
    }

    void update(const dglnet::TextureMessage& msg) {
        std::string errorMsg;
        m_Scene->clear();
        if (!msg.isOk(errorMsg)) {
            m_Scene->addText(errorMsg.c_str());
        } else {
            m_PixelData = std::vector<uchar>(msg.m_Levels[0].m_Pixels.begin(), msg.m_Levels[0].m_Pixels.end());

            uint realHeight = m_PixelData.size() / msg.m_Levels[0].m_Channels / msg.m_Levels[0].m_Width;

            assert(realHeight == msg.m_Levels[0].m_Height);

            QImage::Format format = QImage::Format_Invalid;            
            switch (msg.m_Levels[0].m_Channels) {
                case 4:
                    format = QImage::Format_ARGB32;
                    break;
                case 3:
                    format = QImage::Format_RGB888;
                    break;
                default:
                    assert(0);
            }
            m_Scene->addPixmap(QPixmap::fromImage(QImage(&m_PixelData[0], msg.m_Levels[0].m_Width, realHeight, format)));
        }   
    }

    virtual void requestUpdate(DglController* controller) {
        controller->requestTexture(getObjId(), false);
    }

private: 
    Ui_DGLTextureViewItem m_Ui;
    boost::shared_ptr<QGraphicsScene> m_Scene;
    std::vector<uchar> m_PixelData;
};

DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Textures", "DGLTextureView");
   	
    //inbound
    CONNASSERT(connect(controller, SIGNAL(focusTexture(uint)), this, SLOT(showTexture(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotTexture(uint, const dglnet::TextureMessage&)), this, SLOT(gotTexture(uint, const dglnet::TextureMessage&))));
        
}

void DGLTextureView::showTexture(uint name) {
    update(name);
}

void DGLTextureView::gotTexture(uint name, const dglnet::TextureMessage& msg) {
    DGLTabbedViewItem* widget = getTab(name);
    if (widget) {
        dynamic_cast<DGLTextureViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLTextureView::createTab(uint id) {
    return new DGLTextureViewItem(id, this);
}

QString DGLTextureView::getTabName(uint id, uint target) {
    return QString("Texture ") + QString::number(id);
}