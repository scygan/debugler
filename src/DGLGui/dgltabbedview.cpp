#include "dgltabbedview.h"
#include "dglgui.h"

DGLTabbedViewItem::DGLTabbedViewItem(uint objId, QWidget* parrent):QWidget(parrent),m_ObjId(objId) {}

uint DGLTabbedViewItem::getObjId() {return m_ObjId; }

DGLTabbedView::DGLTabbedView(QWidget* parrent, DglController* controller):QDockWidget(parrent),m_Controller(controller),m_TabWidget(this) {

    m_TabWidget.setTabsClosable(true);
    setWidget(&m_TabWidget);

    m_TabWidget.setEnabled(true);

    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
    CONNASSERT(connect(controller, SIGNAL(setBreaked(bool)), &m_TabWidget, SLOT(setEnabled(bool))));

    //internal
    CONNASSERT(connect(&m_TabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int))));

    m_ResourceManager = controller->getResourceManager();
}


void DGLTabbedView::setConnected(bool connected) {
    if (!connected) {
        m_TabWidget.setEnabled(false);
        while (m_TabWidget.count()) {
            delete m_TabWidget.widget(0);
        }
    }
}

void DGLTabbedView::closeTab(int idx) {
    delete m_TabWidget.widget(idx);
}


void DGLTabbedView::ensureTabDisplayed(uint id, uint target) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTabbedViewItem* widget = dynamic_cast<DGLTabbedViewItem*>(m_TabWidget.widget(i));
        if (widget && widget->getObjId() == id) {
            found = true;
            m_TabWidget.setCurrentIndex(m_TabWidget.indexOf(widget));
        }
    }
    if (!found) {
        m_TabWidget.addTab(createTab(id), getTabName(id, target));
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
