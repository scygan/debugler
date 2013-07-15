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


#ifndef DGLTABBEDVIEW_H
#define DGLTABBEDVIEW_H

#include "dglqtgui.h"
#include <QDockWidget>
#include <QTabWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTabbedViewItem: public QWidget {
public:
    DGLTabbedViewItem(dglnet::ContextObjectName, QWidget* parrent);

    const dglnet::ContextObjectName& getObjName();

private: 
    dglnet::ContextObjectName m_ObjectName;
};

class DGLTabbedView : public QDockWidget {
    Q_OBJECT

public:
    DGLTabbedView(QWidget* parrent, DglController* controller);
    virtual ~DGLTabbedView() {}

    public slots:
        void setConnected(bool);
      
    private slots:
        void closeTab(int);

protected:
    void ensureTabDisplayed(opaque_id_t ctxid, gl_t id, gl_t target = 0);
    DGLTabbedViewItem* getTab(const dglnet::ContextObjectName& id);
    void setupNames(const char* title, const char* objName);

    DglController* m_Controller;
private: 
    virtual DGLTabbedViewItem* createTab(const dglnet::ContextObjectName& id) = 0;
    virtual QString getTabName(gl_t id, gl_t target) = 0;
    QTabWidget m_TabWidget;    
};

#endif // DGLTABBEDVIEW_H
