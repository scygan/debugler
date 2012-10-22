#include "dgltraceview.h"


DGLTraceView::DGLTraceView(QWidget* parrent, DglController* controller):QDockWidget(tr("Call trace"), parrent) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_traceList = new QListWidget(this);
    setWidget(m_traceList);

    assert(connect(controller, SIGNAL(connected()), this, SLOT(clear())));
    assert(connect(controller, SIGNAL(disconnected()), this, SLOT(clear())));
    assert(connect(controller, SIGNAL(breaked(Entrypoint)), this, SLOT(breaked(Entrypoint))));

}


DGLTraceView::~DGLTraceView() {
    delete m_traceList;
}

void DGLTraceView::clear() {
    m_traceList->clear();
}


void DGLTraceView::breaked(Entrypoint entrp) {
    m_traceList->addItems(QStringList() << "Called entrypoint no.: " + QString::number(entrp));
}