#ifndef DGLPIXELRECTANGLE_H
#define DGLPIXELRECTANGLE_H

#include <QGraphicsScene>
#include <QGraphicsView>

#include "ui_dglpixelrectangleview.h"
#include "DGLCommon/gl-formats.h"

class DGLPixRectQGraphicsView: public QGraphicsView {
    Q_OBJECT
public:
    DGLPixRectQGraphicsView(QWidget* parent);
signals:
    void resized(const QSize&);
    void onMouseOver(const QPoint&);
    void onMouseLeft();

private:
    void resizeEvent (QResizeEvent * event);
    void mouseMoveEvent ( QMouseEvent * event );
    void leaveEvent ( QEvent * event );
};


class DGLPixelRectangleScene;

class DGLPixelRectangleView: public QWidget {
    Q_OBJECT
public:
    DGLPixelRectangleView(QWidget* parent, DGLPixelRectangleScene* scene);
    void updateFormatSizeInfo(const DGLPixelRectangle* pixelRectangle);
private slots:
    void onMouseOver(const QPoint& pos);
    void onMouseLeft();
private:
    Ui::DGLPixelRectangleView* m_Ui;
    DGLPixRectQGraphicsView m_GraphicsView;
    DGLPixelRectangleScene* m_Scene;
};

class DGLPixelRectangleBlitter;

class DGLPixelRectangleScene: public QObject {
    Q_OBJECT
public:
    DGLPixelRectangleScene();

    void setText(const std::string& message);
    void setPixelRectangle(const DGLPixelRectangle& pixelRectangle);

    QPoint translate(const QPoint&);
    bool inside(const QPoint&);
    std::pair<QColor, std::vector<AnyValue> > getColor(const QPoint&);

    QGraphicsScene* getScene();

public slots:
    void resize(const QSize&);

private:
    void doRecalcSizes();
    QGraphicsScene m_Scene;
    QGraphicsPixmapItem * m_Item;
    QImage m_Image;
    float m_Scale;
    QPoint m_Pos;

    void blittedImage(const QImage& image);

    boost::shared_ptr<DGLPixelRectangleBlitter> m_Blitter;

    friend class DGLPixelRectangleBlitter;
};


#endif
