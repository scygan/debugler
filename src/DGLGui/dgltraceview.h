#ifndef DGLTRACEVIEW_H
#define DGLTRACEVIEW_H

#include <QDockWidget>
#include <QListWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTraceView : public QDockWidget {
    Q_OBJECT

public:
    DGLTraceView(QWidget* parrent, DglController* controller);
    ~DGLTraceView();

public slots:
    void clear();
    void breaked(Entrypoint);

private: 
    QListWidget *m_traceList;
};

#endif // DGLTRACEVIEW_H