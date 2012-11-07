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
    ~DGLTextureView();

public slots:
    void enable();
    void disable();

private: 
    QTabWidget *m_TabWidget;
    bool m_Enabled;
};

#endif // DGLTRACEVIEW_H