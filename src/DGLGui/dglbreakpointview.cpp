#include "dglbreakpointview.h"
#include "dglgui.h"


DGLBreakPointView::DGLBreakPointView(QWidget* parrent, DglController* controller):QDockWidget(tr("Break Points"), parrent) {
    setObjectName("DGLBreakPointView");
    m_ListWidget = new QListWidget(this);

    disable();
    setWidget(m_ListWidget);
    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    
    //outbound
    
}


DGLBreakPointView::~DGLBreakPointView() {
    delete m_ListWidget;
}

void DGLBreakPointView::enable() {
    m_Enabled = true;
}

void DGLBreakPointView::disable() {
    m_Enabled = false;
}