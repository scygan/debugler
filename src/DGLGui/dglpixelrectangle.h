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

#ifndef DGLPIXELRECTANGLE_H
#define DGLPIXELRECTANGLE_H

#include "dglqtgui.h"
#include <QGraphicsScene>
#include <QGraphicsView>

#include "ui_dglpixelrectangleview.h"

#include <DGLNet/protocol/resource.h>

class DGLPixRectQGraphicsView : public QGraphicsView {
    Q_OBJECT
   public:
    DGLPixRectQGraphicsView(QWidget* parent);
signals:
    void resized(const QSize&);
    void onMouseOver(const QPoint&);
    void onMouseLeft();

   private:
    void resizeEvent(QResizeEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);
};

class DGLPixelRectangleScene;

class DGLPixelRectangleView : public QWidget {
    Q_OBJECT
   public:
    DGLPixelRectangleView(QWidget* parent);
    void setScene(DGLPixelRectangleScene* scene);
    void updateFormatSizeInfo(
        const dglnet::resource::DGLPixelRectangle* pixelRectangle);
   private
slots:
    void onMouseOver(const QPoint& pos);
    void onMouseLeft();
    void showChannelR(bool);
    void showChannelG(bool);
    void showChannelB(bool);
    void showChannelA(bool);
    void showChannelD(bool);
    void showChannelS(bool);

   private:
    Ui::DGLPixelRectangleView* m_Ui;
    DGLPixRectQGraphicsView m_GraphicsView;
    DGLPixelRectangleScene* m_Scene;
};

class DGLPixelRectangleBlitter;

class DGLPixelRectangleScene : public QObject {
    Q_OBJECT
   public:
    DGLPixelRectangleScene();

    void setText(const std::string& message);
    void setPixelRectangle(
        const dglnet::resource::DGLPixelRectangle& pixelRectangle);

    QPoint translate(const QPoint&);
    bool inside(const QPoint&);
    std::pair<QColor, std::vector<AnyValue> > getColor(const QPoint&);

    QGraphicsScene* getScene();

    DGLPixelRectangleBlitter* getBlitter();

   public
slots:
    void resize(const QSize&);

   private:
    void doRecalcSizes();
    QGraphicsScene m_Scene;
    QGraphicsPixmapItem* m_Item;
    QImage m_Image;
    float m_Scale;
    QPoint m_Pos;

    void blittedImage(const QImage& image);

    boost::shared_ptr<DGLPixelRectangleBlitter> m_Blitter;

    friend class DGLPixelRectangleBlitter;
};

#endif
