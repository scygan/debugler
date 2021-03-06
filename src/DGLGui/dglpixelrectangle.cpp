/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "dglpixelrectangle.h"
#include "dglblitterbase.h"
#include <DGLNet/protocol/resource.h>

#include <QGraphicsView>

#pragma warning(push)

// needs to have dll-interface to be used by clients of class
#pragma warning(disable : 4251)
#include <QGraphicsPixmapItem>
#pragma warning(pop)

#include <QResizeEvent>
#include <sstream>

#include <DGLCommon/def.h>

DGLPixRectQGraphicsView::DGLPixRectQGraphicsView(QWidget* _parent)
        : QGraphicsView(_parent) {
    QPixmap* tile = new QPixmap(128, 128);
    tile->fill(Qt::white);
    QPainter pt(tile);
    QColor color(230, 230, 230);
    pt.fillRect(0, 0, 64, 64, color);
    pt.fillRect(64, 64, 64, 64, color);
    pt.end();
    QBrush* brush = new QBrush(*tile);
    setBackgroundBrush(*brush);
    setMouseTracking(true);
}

void DGLPixRectQGraphicsView::resizeEvent(QResizeEvent* _event) {
    emit resized(_event->size());
}
void DGLPixRectQGraphicsView::mouseMoveEvent(QMouseEvent* _event) {
    emit onMouseOver(_event->pos());
    QGraphicsView::mouseMoveEvent(_event);
}
void DGLPixRectQGraphicsView::leaveEvent(QEvent* /*_event*/) {
    emit onMouseLeft();
}

class DGLPixelRectangleBlitter : public DGLBlitterBase {
   public:
    DGLPixelRectangleBlitter(DGLPixelRectangleScene* scene) : m_Scene(scene) {}

   private:
    void sink(int width, int height, OutputFormat format, void* data) {
        QImage::Format qtFormat;
        switch (format) {
            case _GL_BGRA32:
                qtFormat = QImage::Format_ARGB32;
                break;
            case _GL_RGBX32:
                qtFormat = QImage::Format_RGB888;
                break;
            case _GL_MONO8:
                qtFormat = QImage::Format_Indexed8;
                break;
            case _LAST:
            default:
                DGL_ASSERT(0);
                return;
        }
        m_Image = QImage(static_cast<uchar*>(data), width, height, qtFormat);
        if (format == _GL_MONO8) {
            for (int i = 0; i < 256; ++i) {
                m_Image.setColor(i, qRgb(i, i, i));
            }
        }

        m_Scene->blittedImage(m_Image);
    }
    DGLPixelRectangleScene* m_Scene;
    QImage m_Image;
};

DGLPixelRectangleView::DGLPixelRectangleView(QWidget* _parent)
        : m_GraphicsView(_parent), m_Scene(NULL) {
    m_Ui = new Ui::DGLPixelRectangleView();
    m_Ui->setupUi(this);
    m_Ui->verticalLayout->insertWidget(0, &m_GraphicsView);
    m_Ui->widgetColor->setStyleSheet(
            "QWidget{background-color: palette(background);}");

    QPalette pal;
    pal.setColor(m_Ui->widgetColor->backgroundRole(), QColor(0, 0, 0, 0));
    m_Ui->widgetColor->setPalette(pal);

    m_Ui->widgetColor->setAutoFillBackground(true);

    // process Mouse events ourselves
    CONNASSERT(&m_GraphicsView, SIGNAL(onMouseOver(QPoint)), this,
               SLOT(onMouseOver(QPoint)));
    CONNASSERT(&m_GraphicsView, SIGNAL(onMouseLeft()), this,
               SLOT(onMouseLeft()));
}

void DGLPixelRectangleView::setScene(DGLPixelRectangleScene* scene) {
    m_Scene = scene;

    // pass resize events directly to scene for size recalc
    CONNASSERT(&m_GraphicsView, SIGNAL(resized(const QSize&)), scene,
               SLOT(resize(const QSize&)));

    m_GraphicsView.setScene(scene->getScene());
}

void DGLPixelRectangleView::onMouseOver(const QPoint& barePos) {
    QPoint currentPos = m_Scene->translate(barePos);
    if (m_Scene->inside(currentPos)) {

        std::pair<QColor, std::vector<AnyValue> > currColor =
                m_Scene->getColor(currentPos);

        std::ostringstream tmp;
        tmp << "(" << currentPos.x() << ", " << currentPos.y() << ") = [";
        for (size_t i = 0; i < currColor.second.size(); i++) {
            currColor.second[i].writeToSS(tmp, GLParamTypeMetadata());
            if (i != currColor.second.size() - 1) {
                tmp << ", ";
            }
        }
        tmp << "]";

        m_Ui->labelColorText->setText(QString::fromStdString(tmp.str()));
        QPalette pal;
        pal.setColor(m_Ui->widgetColor->backgroundRole(),
                     QColor(currColor.first));
        m_Ui->widgetColor->setPalette(pal);

    } else {
        onMouseLeft();
    }
}

void DGLPixelRectangleView::onMouseLeft() {
    m_Ui->labelColorText->clear();
    QPalette pal;
    pal.setColor(m_Ui->widgetColor->backgroundRole(), QColor(0, 0, 0, 0));
    m_Ui->widgetColor->setPalette(pal);
}

void DGLPixelRectangleView::showChannelR(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_R,
                                           _show ? 1.0f : 0.0f, 0.0f);
}
void DGLPixelRectangleView::showChannelG(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_G,
                                           _show ? 1.0f : 0.0f, 0.0f);
}
void DGLPixelRectangleView::showChannelB(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_B,
                                           _show ? 1.0f : 0.0f, 0.0f);
}
void DGLPixelRectangleView::showChannelA(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_A,
                                           _show ? 1.0f : 0.0f,
                                           _show ? 0.0f : 1.0f);
}
void DGLPixelRectangleView::showChannelD(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_D,
                                           _show ? 1.0f : 0.0f, 0.0f);
}
void DGLPixelRectangleView::showChannelS(bool _show) {
    m_Scene->getBlitter()->setChannelScale(DGLBlitterBase::CHANNEL_S,
                                           _show ? 1.0f : 0.0f, 0.0f);
}

void DGLPixelRectangleView::updateFormatSizeInfo(
        const dglnet::resource::DGLPixelRectangle* pixelRectangle, gl_t format, value_t samples) {
    if (pixelRectangle) {
        std::ostringstream formatSize;
        if (!format) {
            formatSize << "<unknown format>";
        } else {
            formatSize << GetGLEnumName(format, GLEnumGroup::InternalFormat);
        }
        formatSize << " ";
        formatSize << "(" << pixelRectangle->m_Width << "x"
                   << pixelRectangle->m_Height << ") ";
        if (samples) {
            formatSize << "MSAA: " << samples;
        }
        m_Ui->labelFormatSize->setText(
                QString::fromStdString(formatSize.str()));
    } else {
        m_Ui->labelFormatSize->clear();
    }
}

DGLPixelRectangleScene::DGLPixelRectangleScene()
        : m_Item(NULL),
          m_Blitter(std::make_shared<DGLPixelRectangleBlitter>(this)) {}

void DGLPixelRectangleScene::setText(const std::string& message) {
    m_Item = NULL;
    m_Scene.clear();
    m_Scene.addText(message.c_str());
}

void DGLPixelRectangleScene::setPixelRectangle(
        const dglnet::resource::DGLPixelRectangle& pixelRectangle) {

    m_Item = NULL;

    if (!pixelRectangle.getPtr()) {
        setText("No pixels");
        return;
    }

    m_Blitter->blit(pixelRectangle.m_Width, pixelRectangle.m_Height,
                    pixelRectangle.m_RowBytes, pixelRectangle.m_GLFormat,
                    pixelRectangle.m_GLType, pixelRectangle.getPtr());
}

QPoint DGLPixelRectangleScene::translate(const QPoint& pos) {
    return QPoint(((QPointF(pos) - QPointF(m_Pos)) / m_Scale -
                   QPointF(0.5, 0.5)).toPoint());
}

std::pair<QColor, std::vector<AnyValue> > DGLPixelRectangleScene::getColor(
        const QPoint& pos) {
    std::pair<QColor, std::vector<AnyValue> > ret;
    ret.first = m_Image.pixel(pos.x(), pos.y());
    ret.second = m_Blitter->describePixel(pos.x(), pos.y());
    return ret;
}

void DGLPixelRectangleScene::blittedImage(const QImage& image) {
    m_Scene.clear();
    m_Image = image;
    m_Item = m_Scene.addPixmap(QPixmap::fromImage(image));
    doRecalcSizes();
}

QGraphicsScene* DGLPixelRectangleScene::getScene() { return &m_Scene; }

DGLPixelRectangleBlitter* DGLPixelRectangleScene::getBlitter() {
    return m_Blitter.get();
}

void DGLPixelRectangleScene::resize(const QSize& size) {
    m_Scene.setSceneRect(0, 0, size.width(), size.height());
    doRecalcSizes();
}

bool DGLPixelRectangleScene::inside(const QPoint& pos) {
    return pos.x() >= 0 && pos.y() >= 0 && pos.x() < m_Image.size().width() &&
           pos.y() < m_Image.size().height();
}

void DGLPixelRectangleScene::doRecalcSizes() {
    if (!m_Item) return;

    m_Scale = (float)std::min(
            m_Scene.sceneRect().width() / m_Image.size().width(),
            m_Scene.sceneRect().height() / m_Image.size().height());

    // rescale
    m_Item->setScale(m_Scale);

    // move
    QRectF r = m_Item->sceneBoundingRect();
    m_Pos = QPoint(
            static_cast<int>((m_Scene.sceneRect().width() - r.width()) / 2.0 +
                             0.5),
            static_cast<int>((m_Scene.sceneRect().height() - r.height()) / 2.0 +
                             0.5));
    m_Item->setPos(m_Pos);
}
