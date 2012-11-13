#include "dgltextureview.h"
#include "dglgui.h"

#include "ui_dgltextureviewwidget.h"

#include <QGraphicsTextItem>

class DGLTextureViewWidget: public QWidget {
public:
    DGLTextureViewWidget(uint name, QWidget* parrent):QWidget(parrent),m_TextureName(name) {
        m_Ui.setupUi(this);
    }
    uint getTextureName() {return m_TextureName; }

    void update(dglnet::TextureMessage msg) {
        QGraphicsScene scene(0, 0, 100, 100, this);
        std::string errorMsg;
        /*QGraphicsTextItem* item;
        if (!msg.isOk(errorMsg)) {
            item = scene.addText(errorMsg.c_str());
            item->setVisible(true);
        } else {
            std::vector<uchar> data(msg.m_Levels[0].m_Height * msg.m_Levels[0].m_Width * 4); 
            for (size_t i = 0; i < msg.m_Levels[0].m_Height * msg.m_Levels[0].m_Width; i++) {
                int srcIdx = i * msg.m_Levels[0].m_Channels;
                for (int j = 0; j < 3; j++)
                    data[i + j + 1] = (msg.m_Levels[0].m_Channels > j)?msg.m_Levels[0].m_Pixels[srcIdx + j] * 255.f + .5f:0;
                data[i] = (msg.m_Levels[0].m_Channels > 3)?msg.m_Levels[0].m_Pixels[srcIdx + 3] * 255.f + .5f:0xff;
            }
            QImage image(&data[0], msg.m_Levels[0].m_Height, msg.m_Levels[0].m_Width, QImage::Format_ARGB32);
            scene.addPixmap(QPixmap::fromImage(image));
        }*/
        uchar data[4] = { 255, 0, 255, 255};
        QImage image(&data[0], 1, 1,QImage::Format_ARGB32);
        scene.addPixmap(QPixmap::fromImage(image));
        scene.addText("kupa kupa");
        m_Ui.graphicsView->setScene(&scene);
        m_Ui.graphicsView->show();
    }

private: 
    uint m_TextureName;
    Ui_DGLTextureViewWidget m_Ui;
};

DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):QDockWidget(tr("Textures"), parrent),m_Controller(controller) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_TabWidget = new QTabWidget(this);

    disable();
    setWidget(m_TabWidget);
    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    CONNASSERT(connect(controller, SIGNAL(showTexture(uint)), this, SLOT(showTexture(uint))));
    CONNASSERT(connect(controller, SIGNAL(gotTexture(uint, dglnet::TextureMessage)), this, SLOT(gotTexture(uint, dglnet::TextureMessage))));
    
    //outbound
    
}


DGLTextureView::~DGLTextureView() {
    delete m_TabWidget;
}

void DGLTextureView::enable() {
    m_Enabled = true;
}

void DGLTextureView::disable() {
    m_Enabled = false;
}

void DGLTextureView::showTexture(uint name) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget->count(); i++) {
        DGLTextureViewWidget* widget = dynamic_cast<DGLTextureViewWidget*>(m_TabWidget->widget(i));
        if (widget && widget->getTextureName() == name) {
            found = true;
        }
    }
    if (!found) {
        m_TabWidget->addTab(new DGLTextureViewWidget(name, this), QString("Texture ") + QString::number(name));
        
    }
    m_Controller->debugQueryTexture(name);
}

void DGLTextureView::gotTexture(uint name, dglnet::TextureMessage msg) {
    int i; 
    DGLTextureViewWidget* widget;
    for (i = 0; i < m_TabWidget->count(); i++) {
        widget = dynamic_cast<DGLTextureViewWidget*>(m_TabWidget->widget(i));
        if (widget && widget->getTextureName() == name) {
            break;
        }
    }
    if (i != m_TabWidget->count()) {
        widget->update(msg);
    }
}
