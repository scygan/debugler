#include <dglpixelrectangle.h>
#include <DGLCommon/gl-serialized.h>

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>


#include "dglgui.h"

DGLPixelRectangleView::DGLPixelRectangleView(QWidget* parent, DGLPixelRectangleScene* scene):QGraphicsView(parent) {
    QPixmap* tile = new QPixmap(128, 128);
    tile->fill(Qt::white);
    QPainter pt(tile);
    QColor color(230, 230, 230);
    pt.fillRect(0, 0, 64, 64, color);
    pt.fillRect(64, 64, 64, 64, color);
    pt.end();
    QBrush *brush = new QBrush(*tile);
    setBackgroundBrush(*brush);
    setScene(scene->getScene());
    CONNASSERT(connect(this, SIGNAL(resized(const QSize&)), scene, SLOT(resize(const QSize&))));
}


void DGLPixelRectangleView::resizeEvent (QResizeEvent * event) {
    emit resized(event->size());
}


DGLPixelRectangleScene::DGLPixelRectangleScene():m_Item(NULL) {}

void DGLPixelRectangleScene::setText(const std::string& message) {
    m_Item = NULL;
    m_Scene.clear();
    m_Scene.addText(message.c_str());
}

void DGLPixelRectangleScene::setPixelRectangle(const DGLPixelRectangle* pixelRectangle) {

    m_Item = NULL;

    if (!(pixelRectangle->m_Width && pixelRectangle->m_Pixels.size())) {
        setText("Texture empty");
        return;
    }

    if (pixelRectangle->m_Channels == 2) {
        //special case: recompute buffer to be 4 element
        m_PixelData.resize(pixelRectangle->m_Pixels.size() * 2, 0);
        for (size_t i = 0; i < pixelRectangle->m_Pixels.size(); i+=2) {
            m_PixelData[2 * i + 0] =  pixelRectangle->m_Pixels[i + 0];
            m_PixelData[2 * i + 1] = pixelRectangle->m_Pixels[i + 1];
        }
        m_ImageSize.setHeight(m_PixelData.size() / 4 / pixelRectangle->m_Width);
    } else {
        //normal case, use buffer untouched
        m_PixelData = std::vector<uchar>(pixelRectangle->m_Pixels.begin(), pixelRectangle->m_Pixels.end());

        m_ImageSize.setHeight(m_PixelData.size() / pixelRectangle->m_Channels / pixelRectangle->m_Width);
    }

    assert(m_ImageSize.height() == pixelRectangle->m_Height);
    m_ImageSize.setWidth(pixelRectangle->m_Width);

    QImage::Format format = QImage::Format_Invalid;            
    switch (pixelRectangle->m_Channels) {
    case 4:
        format = QImage::Format_ARGB32;
        break;
    case 3:
        format = QImage::Format_RGB888;
        break;
    case 2:
        format = QImage::Format_RGB32; //we have recomputed buffer, so we can use 32-bit non-alpha format
        break;
    case 1:
        format = QImage::Format_Indexed8;
        break;
    default:
        assert(0);
    }

    QImage image(&m_PixelData[0], m_ImageSize.width(), m_ImageSize.height(), format);

    if (pixelRectangle->m_Channels == 1) {
        //this is special case: we do not want to re-compute buffer, so we use color indices to emulate GL_R8 texture
        for(int i=0;i<256;++i) {
            image.setColor(i, qRgb(i,i,i));
        }
    }

    m_Scene.clear();
    m_Item = m_Scene.addPixmap(QPixmap::fromImage(image));

    doRecalcSizes();
}

QGraphicsScene* DGLPixelRectangleScene::getScene() {
    return &m_Scene;
}

void DGLPixelRectangleScene::resize(const QSize& size) {
    m_Scene.setSceneRect(0, 0, size.width(), size.height());
    doRecalcSizes();
}

void DGLPixelRectangleScene::doRecalcSizes() {
    if (!m_Item) return;

    //rescale
    m_Item->setScale(min(m_Scene.sceneRect().width() / m_ImageSize.width(),
        m_Scene.sceneRect().height() / m_ImageSize.height()));
    
    //move
    QRectF r = m_Item->sceneBoundingRect();
    float yPos = (m_Scene.sceneRect().height() - r.height()) / 2.0;
    float xPos = (m_Scene.sceneRect().width() - r.width()) / 2.0;
    m_Item->setPos(xPos, yPos);
}