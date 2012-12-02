#ifndef DGLTRACEVIEW_H
#define DGLTRACEVIEW_H

#include <QDockWidget>
#include <QListWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTraceViewList: public QListWidget {
    Q_OBJECT
public:
    DGLTraceViewList(QWidget*);
    uint getVisibleRowCount(); 
    uint getFirstVisibleElementIdx(); 

private:
    virtual void resizeEvent (QResizeEvent* e);
signals:
    void resized();
};


class DGLTraceView : public QDockWidget {
    Q_OBJECT

public:
    DGLTraceView(QWidget* parrent, DglController* controller);

signals: 
    void queryCallTrace(uint startOffset, uint endOffset);

public slots:
    void setEnabled(bool);
    void setRunning(bool);
    void breaked(CalledEntryPoint, uint);
    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);


    void mayNeedNewElements();

private: 
    DGLTraceViewList m_traceList;
    bool m_Enabled;
    int m_QueryUpperBound;
};

#endif // DGLTRACEVIEW_H