#include "dgltextureview.h"
#include "dglgui.h"


DGLTextureView::DGLTextureView(QWidget* parrent, DglController* controller):QDockWidget(tr("Textures"), parrent) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_TabWidget = new QTabWidget(this);

    disable();
    setWidget(m_TabWidget);
    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    
    //outbound
    
}


DGLTextureView::~DGLTextureView() {
    delete m_TabWidget;
}

void DGLTextureView::enable() {
    m_Enabled = true;
}

void DGLTextureView::disable() {
    m_Enabled = false;
}