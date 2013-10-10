/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "dgltabbedview.h"

DGLTabbedViewItem::DGLTabbedViewItem(dglnet::ContextObjectName objName, QWidget* parrent):QWidget(parrent),m_ObjectName(objName) {}

const dglnet::ContextObjectName& DGLTabbedViewItem::getObjName() {return m_ObjectName; }

DGLTabbedView::DGLTabbedView(QWidget* parrent, DglController* controller):QDockWidget(parrent),m_Controller(controller),m_TabWidget(this) {

    m_TabWidget.setTabsClosable(true);
    setWidget(&m_TabWidget);

    m_TabWidget.setEnabled(true);

    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
    CONNASSERT(connect(controller, SIGNAL(setBreaked(bool)), &m_TabWidget, SLOT(setEnabled(bool))));

    //internal
    CONNASSERT(connect(&m_TabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int))));
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


void DGLTabbedView::ensureTabDisplayed(opaque_id_t ctxId, gl_t objName, gl_t target) {
    bool found = false; 
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTabbedViewItem* itemWidget = dynamic_cast<DGLTabbedViewItem*>(m_TabWidget.widget(i));
        if (itemWidget && itemWidget->getObjName().m_Context == ctxId &&
            itemWidget->getObjName().m_Name == objName &&
            itemWidget->getObjName().m_Target == target) {
            found = true;
            m_TabWidget.setCurrentIndex(m_TabWidget.indexOf(itemWidget));
        }
    }
    if (!found) {
        m_TabWidget.addTab(createTab(dglnet::ContextObjectName(ctxId, objName, target)), getTabName(objName, target));
        m_TabWidget.setCurrentIndex(m_TabWidget.count() - 1);
    }
    raise();
}

DGLTabbedViewItem* DGLTabbedView::getTab(const dglnet::ContextObjectName& id) {
    for (int i = 0; i < m_TabWidget.count(); i++) {
        DGLTabbedViewItem* itemWidget = dynamic_cast<DGLTabbedViewItem*>(m_TabWidget.widget(i));
        if (itemWidget && itemWidget->getObjName() == id) {
            return itemWidget;
        }
    }
    return NULL;
}

void DGLTabbedView::setupNames(const char* title, const char* objName) {
    setObjectName(objName);
    setWindowTitle(title);
}
