#include "dgltabbedview.h"
#include "dglgui.h"

DGLTabbedViewItem::DGLTabbedViewItem(uint objId, QWidget* parrent):QWidget(parrent),m_ObjId(objId) {}

uint DGLTabbedViewItem::getObjId() {return m_ObjId; }

DGLTabbedView::DGLTabbedView(QWidget* parrent, DglController* controller):QDockWidget(parrent),m_Controller(controller),m_TabWidget(this) {

    disable();
    m_TabWidget.setTabsClosable(true);
    setWidget(&m_TabWidget);

    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));

    //internal
    CONNASSERT(connect(&m_TabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int))));
}

void DGLTabbedView::enable() {
    m_Enabled = true;
}

void DGLTabbedView::disable() {
    m_Enabled = false;
    while (m_TabWidget.count()) {
        delete m_TabWidget.widget(0);
    }
}

void DGLTabbedView::closeTab(int idx) {
    delete m_TabWidget.widget(idx);
}


void DGLTabbedView::update(uint id) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTabbedViewItem* widget = dynamic_cast<DGLTabbedViewItem*>(m_TabWidget.widget(i));
        if (widget && widget->getObjId() == id) {
            found = true;
            m_TabWidget.setCurrentIndex(m_TabWidget.indexOf(widget));
        }
    }
    if (!found) {
        m_TabWidget.addTab(createTab(id), getTabName(id));
        m_TabWidget.setCurrentIndex(m_TabWidget.count() - 1);
    }
    raise();
}

DGLTabbedViewItem* DGLTabbedView::getTab(uint id) {
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTabbedViewItem* widget = dynamic_cast<DGLTabbedViewItem*>(m_TabWidget.widget(i));
        if (widget && widget->getObjId() == id) {
            return widget;
        }
    }
    return NULL;
}

void DGLTabbedView::setupNames(char* title, char* objName) {
    setObjectName(objName);
    setWindowTitle(title);
}