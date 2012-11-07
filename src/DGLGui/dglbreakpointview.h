#ifndef DGKBREAKPOINTVIEW_H
#define DGKBREAKPOINTVIEW_H

#include <QDockWidget>
#include <QListWidget>

#include <DGLCommon/gl-types.h>

#include "dglcontroller.h"

class DGLBreakPointView : public QDockWidget {
    Q_OBJECT

public:
    DGLBreakPointView(QWidget* parrent, DglController* controller);
    ~DGLBreakPointView();

public slots:
    void enable();
    void disable();

private: 
    QListWidget *m_ListWidget;
    bool m_Enabled;
};

#endif // DGLTRACEVIEW_H