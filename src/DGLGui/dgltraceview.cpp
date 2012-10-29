#include "dgltraceview.h"

#include <QScrollBar>

DGLTraceViewList::DGLTraceViewList(QWidget* parrent):QListWidget(parrent) {
    assert(connect(this, SIGNAL(resized()), parrent, SLOT(mayNeedNewElements())));
    assert(connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), parrent, SLOT(mayNeedNewElements())));
}

uint DGLTraceViewList::getVisibleRowCount() {
    QListWidgetItem* minimumItem = itemAt(5, 5);
    QListWidgetItem* maximumItem = itemAt(5, height() - 5);
    if ( !minimumItem ) { minimumItem = item(0); }
    if ( !maximumItem ) { maximumItem = item(count() - 1); }
    return indexFromItem(maximumItem).row() - indexFromItem(minimumItem).row() + 1;
}

uint DGLTraceViewList::getFirstVisibleElementIdx() {
    QListWidgetItem* minimumItem = itemAt(5, 5);
    if ( !minimumItem ) { minimumItem = item(0); }
    return indexFromItem(minimumItem).row();
}

void DGLTraceViewList::resizeEvent (QResizeEvent* e) {
    resized();
}

DGLTraceView::DGLTraceView(QWidget* parrent, DglController* controller):QDockWidget(tr("Call trace"), parrent) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_traceList = new DGLTraceViewList(this);

    disable();

    setWidget(m_traceList);
    //inbound
    assert(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    assert(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    assert(connect(controller, SIGNAL(breaked(CalledEntryPoint, uint)), this, SLOT(breaked(CalledEntryPoint, uint))));
    assert(connect(controller, SIGNAL(gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&)), this, SLOT(gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&))));
    //outbound
    assert(connect(this, SIGNAL(queryCallTrace(uint, uint)), controller, SLOT(queryCallTrace(uint, uint))));
}


DGLTraceView::~DGLTraceView() {
    delete m_traceList;
}

void DGLTraceView::enable() {
    m_traceList->clear();
    m_Enabled = true;
    m_QueryUpperBound = 0;
}

void DGLTraceView::disable() {
    m_traceList->clear();
    m_Enabled = false;
    m_QueryUpperBound = 0;
}

void DGLTraceView::mayNeedNewElements() {
    if (m_Enabled) {
        int visibleRows = m_traceList->getVisibleRowCount();
        if (m_traceList->getFirstVisibleElementIdx() < m_traceList->count() - m_QueryUpperBound - 1) {
            //we are starving of entrypoints to display, try to query new entrypoints up to this bound
            int nextUpperBound = m_QueryUpperBound + 2 * m_traceList->getVisibleRowCount();
            queryCallTrace(m_QueryUpperBound, nextUpperBound);
            m_QueryUpperBound = nextUpperBound;
        }
    }
}

void DGLTraceView::breaked(CalledEntryPoint entryp, uint traceSize) {
    m_traceList->clear();
    for (uint i = 0; i < traceSize; i++) {
        m_traceList->addItem(QString("<unknown>"));
    }
    m_traceList->addItems(QStringList() << QString("Breaked on: ") + GetEntryPointName(entryp.getEntrypoint()));
    m_traceList->setCurrentRow(m_traceList->count() - 1);
    m_traceList->scrollToBottom();
    m_QueryUpperBound = 0;
    mayNeedNewElements();
}

void DGLTraceView::gotCallTraceChunkChunk(uint offset, const std::vector<CalledEntryPoint>& trace) {
    for (uint i = offset; i < offset + trace.size(); i++) {
        int row = m_traceList->count() - i - 2;
        delete m_traceList->takeItem(row);
        m_traceList->insertItem(row, QString("Older: ") + GetEntryPointName(trace[trace.size() - 1 - i + offset].getEntrypoint()));
    }
}