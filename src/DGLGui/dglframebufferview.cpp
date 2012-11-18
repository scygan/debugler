#include "dglframebufferview.h"
#include "dglgui.h"
#include "QHexEdit/qhexedit.h"

#include "ui_dglframebufferviewitem.h"

class DGLFramebufferViewItem: public DGLTabbedViewItem {
public:
    DGLFramebufferViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent) {
        m_Ui.setupUi(this);
        m_Scene = boost::make_shared<QGraphicsScene>(this);
        m_Scene->setSceneRect(0, 0, 400, 320);
        m_Ui.graphicsView->setScene(m_Scene.get());
        m_Ui.graphicsView->show();
    }

    void update(const dglnet::FramebufferMessage& msg) {
        std::string errorMsg;
        m_Scene->clear();
        if (!msg.isOk(errorMsg)) {
            m_Scene->addText(errorMsg.c_str());
        } else {
            m_PixelData = std::vector<uchar>(msg.m_Pixels.begin(), msg.m_Pixels.end());

            uint realHeight = m_PixelData.size() / msg.m_Channels / msg.m_Width;

            assert(realHeight == msg.m_Height);

            QImage::Format format = QImage::Format_Invalid;            
            switch (msg.m_Channels) {
            case 4:
                format = QImage::Format_ARGB32;
                break;
            case 3:
                format = QImage::Format_RGB888;
                break;
            default:
                assert(0);
            }
            m_Scene->addPixmap(QPixmap::fromImage(QImage(&m_PixelData[0], msg.m_Width, realHeight, format)));
        }
    }
private: 
    Ui_DGLFramebufferViewItem m_Ui;
    boost::shared_ptr<QGraphicsScene> m_Scene;
    std::vector<uchar> m_PixelData;
};

DGLFramebufferView::DGLFramebufferView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Frame Buffers", "DGLFramebufferView");

    //inbound
    CONNASSERT(connect(controller, SIGNAL(showFramebuffer(uint)), this, SLOT(showFramebuffer(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotFramebuffer(uint, const dglnet::FramebufferMessage&)), this, SLOT(gotFramebuffer(uint, const dglnet::FramebufferMessage&))));
}

void DGLFramebufferView::showFramebuffer(uint bufferEnum) {
    update(bufferEnum);
    m_Controller->debugQueryFramebuffer(bufferEnum);
}

void DGLFramebufferView::gotFramebuffer(uint bufferEnum, const dglnet::FramebufferMessage& msg) {
    DGLTabbedViewItem* widget = getTab(bufferEnum);
    if (widget) {
        dynamic_cast<DGLFramebufferViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLFramebufferView::createTab(uint id) {
    return new DGLFramebufferViewItem(id, this);
}

QString DGLFramebufferView::getTabName(uint id) {
    return QString("Frame Buffer ") + QString::number(id);
}