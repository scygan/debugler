#ifndef DGLPIXELRECTANGLE_H
#define DGLPIXELRECTANGLE_H

#include <QGraphicsScene>
#include <QGraphicsView>

#include "ui_dglpixelrectangleview.h"

class DGLPixelRectangleScene;

class DGLPixelRectangleViewWidget: public QGraphicsView {
    Q_OBJECT
public:
    DGLPixelRectangleViewWidget(QWidget* parent, DGLPixelRectangleScene* scene);
signals:
    void resized(const QSize&);
    void onMouseOver(const QPoint& pos, const QColor& color, int numChannels);
    void onMouseLeft();

private:
    virtual void resizeEvent (QResizeEvent * event);
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void leaveEvent ( QEvent * event );
    DGLPixelRectangleScene* m_Scene;
};

class DGLPixelRectangle;

class DGLPixelRectangleView: public QGraphicsView {
    Q_OBJECT
public:
    DGLPixelRectangleView(QWidget* parent, DGLPixelRectangleScene* scene);
    void updateFormatSizeInfo(const DGLPixelRectangle* pixelRectangle);
private slots:
    void onMouseOver(const QPoint& pos, const QColor& color, int numChannels);
    void onMouseLeft();
private:
    Ui::DGLPixelRectangleView* m_Ui;
    DGLPixelRectangleViewWidget m_ViewWidget;
};

class QGraphicsPixmapItem;

class DGLPixelRectangleScene: public QObject {
    Q_OBJECT
public:
    DGLPixelRectangleScene();

    void setText(const std::string& message);
    void setPixelRectangle(const DGLPixelRectangle* pixelRectangle);
    QGraphicsScene* getScene();
    float getScale();
    const QPoint& getPos();
    const QSize& getImageSize();
    QColor getColor(int x, int y);
    int getNumChannels();

public slots:
    void resize(const QSize&);

private:
    void doRecalcSizes();

    std::vector<uchar> m_PixelData;
    QGraphicsScene m_Scene;
    QGraphicsPixmapItem * m_Item;
    QSize m_ImageSize;
    float m_Scale;
    QPoint m_Pos;
    size_t m_PixelSize, m_Channels;
};


#endif