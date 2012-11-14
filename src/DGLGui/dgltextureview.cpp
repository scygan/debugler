#include "dgltextureview.h"
#include "dglgui.h"

#include "ui_dgltextureviewwidget.h"

#include <QGraphicsTextItem>

class DGLTextureViewWidget: public QWidget {
public:
    DGLTextureViewWidget(uint name, QWidget* parrent):QWidget(parrent),m_TextureName(name) {
        m_Ui.setupUi(this);
        m_Scene = boost::make_shared<QGraphicsScene>(this);
        m_Scene->setSceneRect(0, 0, 400, 320);
        m_Ui.graphicsView->setScene(m_Scene.get());
        m_Ui.graphicsView->show();
        
    }
    uint getTextureName() {return m_TextureName; }

    void update(dglnet::TextureMessage msg) {
        std::string errorMsg;
        QGraphicsTextItem* item;
        m_Scene->clear();
        if (!msg.isOk(errorMsg)) {
            item = m_Scene->addText(errorMsg.c_str());
        } else {
            std::vector<uchar> data(msg.m_Levels[0].m_Height * msg.m_Levels[0].m_Width * 4); 
            for (size_t i = 0; i < msg.m_Levels[0].m_Height * msg.m_Levels[0].m_Width; i++) {
                int srcIdx = i * msg.m_Levels[0].m_Channels;
                data[4 * i + 0] = (msg.m_Levels[0].m_Channels >= 3)?msg.m_Levels[0].m_Pixels[srcIdx + 2] * 255.f + .5f:0;
                data[4 * i + 1] = (msg.m_Levels[0].m_Channels >= 2)?msg.m_Levels[0].m_Pixels[srcIdx + 1] * 255.f + .5f:0;
                data[4 * i + 2] = (msg.m_Levels[0].m_Channels >= 1)?msg.m_Levels[0].m_Pixels[srcIdx + 0] * 255.f + .5f:0;
                data[4 * i + 3] = (msg.m_Levels[0].m_Channels >= 4)?msg.m_Levels[0].m_Pixels[srcIdx + 3] * 255.f + .5f:0;
            }
            QImage image(&data[0], msg.m_Levels[0].m_Height, msg.m_Levels[0].m_Width, QImage::Format_ARGB32);
            m_Scene->addPixmap(QPixmap::fromImage(image));
        }
    }

private: 
    uint m_TextureName;
    Ui_DGLTextureViewWidget m_Ui;
    boost::shared_ptr<QGraphicsScene> m_Scene;
};

DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):QDockWidget(tr("Textures"), parrent),m_Controller(controller),m_TabWidget(this) {
    setObjectName("DGLTextureView");

    m_TabWidget.setTabsClosable(true);

    disable();
    setWidget(&m_TabWidget);
    	
    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    CONNASSERT(connect(controller, SIGNAL(showTexture(uint)), this, SLOT(showTexture(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotTexture(uint, dglnet::TextureMessage)), this, SLOT(gotTexture(uint, dglnet::TextureMessage))));
    
    //internal
    CONNASSERT(connect(&m_TabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int))));
}

void DGLTextureView::enable() {
    m_Enabled = true;
}

void DGLTextureView::disable() {
    m_Enabled = false;
    while (m_TabWidget.count()) {
        delete m_TabWidget.widget(0);
    }
}

void DGLTextureView::showTexture(uint name) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTextureViewWidget* widget = dynamic_cast<DGLTextureViewWidget*>(m_TabWidget.widget(i));
        if (widget && widget->getTextureName() == name) {
            found = true;
            m_TabWidget.setCurrentIndex(m_TabWidget.indexOf(widget));
        }
    }
    if (!found) {
        m_TabWidget.addTab(new DGLTextureViewWidget(name, this), QString("Texture ") + QString::number(name));
        m_TabWidget.setCurrentIndex(m_TabWidget.count() - 1);
        
    }
    m_Controller->debugQueryTexture(name);
}

void DGLTextureView::gotTexture(uint name, dglnet::TextureMessage msg) {
    int i; 
    DGLTextureViewWidget* widget;
    for (i = 0; i < m_TabWidget.count(); i++) {
        widget = dynamic_cast<DGLTextureViewWidget*>(m_TabWidget.widget(i));
        if (widget && widget->getTextureName() == name) {
            break;
        }
    }
    if (i != m_TabWidget.count()) {
        widget->update(msg);
    }
}

void DGLTextureView::closeTab(int idx) {
    delete m_TabWidget.widget(idx);
}
