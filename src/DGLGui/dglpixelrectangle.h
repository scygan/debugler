#ifndef DGLPIXELRECTANGLE_H
#define DGLPIXELRECTANGLE_H

#include <QGraphicsScene>
#include <QGraphicsView>

class DGLPixelRectangleScene;

class DGLPixelRectangleView: public QGraphicsView {
    Q_OBJECT
public:
    DGLPixelRectangleView(QWidget* parent, DGLPixelRectangleScene* scene);
signals:
    void resized(const QSize&);

private:
    virtual void resizeEvent (QResizeEvent * event);
};

class DGLPixelRectangle;
class QGraphicsPixmapItem;

class DGLPixelRectangleScene: public QObject {
    Q_OBJECT
public:
    DGLPixelRectangleScene();

    void setText(const std::string& message);
    void setPixelRectangle(const DGLPixelRectangle* pixelRectangle);
    QGraphicsScene* getScene();

public slots:
    void resize(const QSize&);

private:
    void doRecalcSizes();

    std::vector<uchar> m_PixelData;
    QGraphicsScene m_Scene;
    QGraphicsPixmapItem * m_Item;
    QSize m_ImageSize;
};


#endif