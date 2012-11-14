#ifndef DGLTEXTUREVIEW_H
#define DGLTEXTUREVIEW_H

#include <QDockWidget>
#include <QTabWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"


class DGLTextureView : public QDockWidget {
    Q_OBJECT

public:
    DGLTextureView(QWidget* parrent, DglController* controller);

public slots:
    void enable();
    void disable();
    void showTexture(uint);
    void gotTexture(uint, dglnet::TextureMessage);

private slots:
    void closeTab(int);

private: 
    QTabWidget m_TabWidget;
    bool m_Enabled;
    DglController* m_Controller;
};

#endif // DGLTRACEVIEW_H