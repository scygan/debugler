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

#ifndef DGLTREEVIEW_H
#define DGLTREEVIEW_H

#include "dglqtgui.h"
#include <QDockWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

class DGLTreeView;

class QClickableTreeWidgetItem : public QTreeWidgetItem {
   public:
    virtual void handleDoubleClick(DglController*);
};

class DGLTreeView : public QDockWidget {
    Q_OBJECT

   public:
    DGLTreeView(QWidget* parrent, DglController* controller);
    ~DGLTreeView();

    void regiSterItem(QTreeWidgetItem item);

   public
slots:
    void setConnected(bool);
    void debugeeInfo(const std::string&);
    void breakedWithStateReports(
            opaque_id_t currentContextId,
            const std::vector<dglnet::message::utils::ContextReport>&);

    void onDoubleClicked(QTreeWidgetItem*, int);

   private:
    QTreeWidget m_TreeWidget;
    bool m_Connected;
    DglController* m_controller;
};

#endif    // DGLTREEVIEW_H
