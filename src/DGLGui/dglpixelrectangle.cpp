#include <dglpixelrectangle.h>
#include <DGLCommon/gl-serialized.h>

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <sstream>

#include "dglgui.h"

DGLPixelRectangleView::DGLPixelRectangleView(QWidget* parent, DGLPixelRectangleScene* scene):m_ViewWidget(parent, scene) {
    m_Ui = new Ui::DGLPixelRectangleView();
    m_Ui->setupUi(this);
    m_Ui->verticalLayout->insertWidget(0, &m_ViewWidget);
    m_Ui->widgetColor->setStyleSheet("QWidget{background-color: palette(background);}");
    
    QPalette pal;
    pal.setColor(m_Ui->widgetColor->backgroundRole(), QColor(0, 0, 0, 0));
    m_Ui->widgetColor->setPalette(pal);


    m_Ui->widgetColor->setAutoFillBackground(true);

    CONNASSERT(connect(&m_ViewWidget, SIGNAL(onMouseOver(const QPoint&, const QColor&, int)), this, SLOT(onMouseOver(const QPoint&, const QColor&, int))));
    CONNASSERT(connect(&m_ViewWidget, SIGNAL(onMouseLeft()), this, SLOT(onMouseLeft())));
}

void DGLPixelRectangleView::onMouseOver(const QPoint& pos, const QColor& color, int numChannels) {
    std::ostringstream tmp;
    tmp << "(" << pos.x() << ", " << pos.y() << ") = [";
    if (numChannels) {
        tmp << color.red();
        if (numChannels > 1) {
            tmp << ", " << color.green();
            if (numChannels > 2) {
                tmp << ", " << color.blue();
                if (numChannels > 3) {
                    tmp << ", " << color.alpha();
                }
            }
        }
    }
    tmp << "]";
    m_Ui->labelColorText->setText(QString::fromStdString(tmp.str()));
    QPalette pal;
    pal.setColor(m_Ui->widgetColor->backgroundRole(), QColor(color));
    m_Ui->widgetColor->setPalette(pal);
}

void DGLPixelRectangleView::onMouseLeft() {
    m_Ui->labelColorText->clear();
    QPalette pal;
    pal.setColor(m_Ui->widgetColor->backgroundRole(), QColor(0, 0, 0, 0));
    m_Ui->widgetColor->setPalette(pal);
}

void DGLPixelRectangleView::updateFormatSizeInfo(const DGLPixelRectangle* pixelRectangle) {
    if (pixelRectangle) {
        std::ostringstream formatSize;
        if (pixelRectangle->m_InternalFormat)
            formatSize << GetGLEnumName(pixelRectangle->m_InternalFormat) << " ";
        formatSize << "(" << pixelRectangle->m_Width << "x" << pixelRectangle->m_Height << ")";
        m_Ui->labelFormatSize->setText(QString::fromStdString(formatSize.str()));
    } else {
        m_Ui->labelFormatSize->clear();
    }
}

DGLPixelRectangleViewWidget::DGLPixelRectangleViewWidget(QWidget* parent, DGLPixelRectangleScene* scene):QGraphicsView(parent), m_Scene(scene) {
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

    setMouseTracking(true);

    CONNASSERT(connect(this, SIGNAL(resized(const QSize&)), scene, SLOT(resize(const QSize&))));
}

void DGLPixelRectangleViewWidget::resizeEvent (QResizeEvent * event) {
    emit resized(event->size());
}

void DGLPixelRectangleViewWidget::mouseMoveEvent ( QMouseEvent * event ) {
        
    QPoint pos = QPoint(((event->posF() - QPointF(m_Scene->getPos())) / m_Scene->getScale()).toPoint());
    if (pos.x() >= 0 && pos.y() >= 0 && pos.x() < m_Scene->getImageSize().width() && pos.y() < m_Scene->getImageSize().height()) {
        emit onMouseOver(pos, m_Scene->getColor(pos.x(), pos.y()), m_Scene->getNumChannels());
    } else {
        emit onMouseLeft();
    }
    
    QGraphicsView::mouseMoveEvent(event);
}

void DGLPixelRectangleViewWidget::leaveEvent ( QEvent * event ) {
    emit onMouseLeft();
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

    m_Channels = pixelRectangle->m_Channels;

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
        m_PixelSize = 4;
        break;
    case 3:
        format = QImage::Format_RGB888;
        m_PixelSize = 3;
        break;
    case 2:
        format = QImage::Format_RGB32; //we have recomputed buffer, so we can use 32-bit non-alpha format
        m_PixelSize = 4;
        break;
    case 1:
        format = QImage::Format_Indexed8;
        m_PixelSize = 1;
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

float DGLPixelRectangleScene::getScale() {
    return m_Scale;
}

const QPoint& DGLPixelRectangleScene::getPos() {
    return m_Pos;
}

QColor DGLPixelRectangleScene::getColor(int x, int y) {
    int idx = (y * m_ImageSize.width() + x) * m_PixelSize;

    int r = 0, g = 0, b = 0, a = 255;

    if (idx + m_PixelSize <= m_PixelData.size()) {
       switch (m_Channels) {
           case 4:
               r = m_PixelData[idx + 2];
               g = m_PixelData[idx + 1];
               b = m_PixelData[idx + 0];
               a = m_PixelData[idx + 3];
               break;
           case 3:
               r = m_PixelData[idx + 0];
               g = m_PixelData[idx + 1];
               b = m_PixelData[idx + 2];
               break;
           case 2:
               r = m_PixelData[idx + 0];
               g = m_PixelData[idx + 1];
               break;
           case 1:
               r = m_PixelData[idx];
               g = m_PixelData[idx];
               b = m_PixelData[idx];
               break;
       }
    }

    return QColor(r, g, b, a);
}

int DGLPixelRectangleScene::getNumChannels() {
    return m_Channels;
}

void DGLPixelRectangleScene::resize(const QSize& size) {
    m_Scene.setSceneRect(0, 0, size.width(), size.height());
    doRecalcSizes();
}

const QSize& DGLPixelRectangleScene::getImageSize() {
    return m_ImageSize;
}

void DGLPixelRectangleScene::doRecalcSizes() {
    if (!m_Item) return;

    m_Scale = min(m_Scene.sceneRect().width() / m_ImageSize.width(),
        m_Scene.sceneRect().height() / m_ImageSize.height());

    //rescale
    m_Item->setScale(m_Scale);

    //move
    QRectF r = m_Item->sceneBoundingRect();
    m_Pos = QPoint((m_Scene.sceneRect().width() - r.width()) / 2.0, (m_Scene.sceneRect().height() - r.height()) / 2.0);
    m_Item->setPos(m_Pos);
}