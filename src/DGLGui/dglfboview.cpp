#include "DGLFBOView.h"
#include "dglgui.h"

#include "ui_dglfboviewitem.h"


DGLFBOViewItem::DGLFBOViewItem(uint name, QWidget* parrent):DGLTabbedViewItem(name, parrent),m_Error(false) {
    m_Ui.setupUi(this);
    m_Scene = boost::make_shared<QGraphicsScene>(this);
    m_Scene->setSceneRect(0, 0, 400, 320);
    m_Ui.graphicsView->setScene(m_Scene.get());
    m_Ui.graphicsView->show();

    //internal
    CONNASSERT(connect(m_Ui.m_AttListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showAttachment(int))));

}

void DGLFBOViewItem::update(const dglnet::FBOMessage& msg) {
    std::string errorMsg;
    m_Scene->clear();
    m_Ui.m_AttListWidget->clear();
    if (!msg.isOk(errorMsg)) {
        m_Scene->addText(errorMsg.c_str());
        m_Error = true;
    } else {
        m_Error = false;
        m_Attachments = msg.m_Attachments;
        for (int i = 0; i < msg.m_Attachments.size(); i++) {
            m_Ui.m_AttListWidget->addItem(QString(GetGLEnumName(msg.m_Attachments[i].m_Id)));
        }
        showAttachment(0);
    }
}

void DGLFBOViewItem::requestUpdate(DglController* controller) {
    controller->requestFBO(getObjId());
}

void DGLFBOViewItem::showAttachment(int id) {
    if (m_Error || id >= m_Attachments.size() || id < 0) return;

    m_Scene->clear();

    std::string errorMsg;

    if (!m_Attachments[id].isOk(errorMsg)) {
        m_Scene->addText(errorMsg.c_str());
    } else {
        uint realHeight = m_Attachments[id].m_Pixels.size() / m_Attachments[id].m_Channels / m_Attachments[id].m_Width;

        assert(realHeight == m_Attachments[id].m_Height);

        QImage::Format format = QImage::Format_Invalid;            
        switch (m_Attachments[id].m_Channels) {
        case 4:
            format = QImage::Format_ARGB32;
            break;
        case 3:
            format = QImage::Format_RGB888;
            break;
        default:
            assert(0);
        }
        m_Scene->addPixmap(QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(&m_Attachments[id].m_Pixels[0]), m_Attachments[id].m_Width, realHeight, format)));
    }
}


DGLFBOView::DGLFBOView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Framebuffer Objects", "DGLFBOView");

    //inbound
    CONNASSERT(connect(controller, SIGNAL(focusFBO(uint)), this, SLOT(showFBO(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotFBO(uint, const dglnet::FBOMessage&)), this, SLOT(gotFBO(uint, const dglnet::FBOMessage&))));
}

void DGLFBOView::showFBO(uint bufferEnum) {
    update(bufferEnum);
}

void DGLFBOView::gotFBO(uint bufferEnum, const dglnet::FBOMessage& msg) {
    DGLTabbedViewItem* widget = getTab(bufferEnum);
    if (widget) {
        dynamic_cast<DGLFBOViewItem*>(widget)->update(msg);
    }
}

DGLTabbedViewItem* DGLFBOView::createTab(uint id) {
    return new DGLFBOViewItem(id, this);
}

QString DGLFBOView::getTabName(uint id, uint target) {
    return QString("FBO ") + QString::number(id);
}