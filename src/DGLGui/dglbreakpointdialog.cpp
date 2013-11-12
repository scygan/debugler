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

#include "dglbreakpointdialog.h"

class DGLBreakPointDialogItem : public QListWidgetItem {
   public:
    DGLBreakPointDialogItem(Entrypoint e, QListWidget* parrent)
            : QListWidgetItem(QString(GetEntryPointName(e)), parrent),
              m_Entrypoint(e) {}
    Entrypoint get() { return m_Entrypoint; }

   private:
    Entrypoint m_Entrypoint;
};

DGLBreakPointDialog::DGLBreakPointDialog(DglController* controller)
        : m_Controller(controller) {
    m_Ui.setupUi(this);
    m_Ui.leftListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    m_Ui.rightListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    for (Entrypoint i = 0; i < NUM_ENTRYPOINTS; i++) {
        new DGLBreakPointDialogItem(i, m_Ui.leftListWidget);
    }

    std::set<Entrypoint> currentBreakPoints =
        m_Controller->getBreakPoints()->getCurrent();
    for (std::set<Entrypoint>::iterator i = currentBreakPoints.begin();
         i != currentBreakPoints.end(); i++) {
        new DGLBreakPointDialogItem(*i, m_Ui.rightListWidget);
    }
}

DGLBreakPointDialog::~DGLBreakPointDialog() {}

std::set<Entrypoint> DGLBreakPointDialog::getBreakPoints() {
    std::set<Entrypoint> ret;
    for (int i = 0; i < m_Ui.rightListWidget->count(); i++) {
        DGLBreakPointDialogItem* widget =
            dynamic_cast<DGLBreakPointDialogItem*>(
                m_Ui.rightListWidget->item(i));
        assert(widget);
        ret.insert(widget->get());
    }
    return ret;
}

void DGLBreakPointDialog::addBreakPoint() {
    QList<QListWidgetItem*> list = m_Ui.leftListWidget->selectedItems();

    for (int i = 0; i < list.count(); i++) {
        DGLBreakPointDialogItem* widget1 =
            dynamic_cast<DGLBreakPointDialogItem*>(list.at(i));
        assert(widget1);
        bool found = false;
        for (int j = 0; j < m_Ui.rightListWidget->count(); j++) {
            DGLBreakPointDialogItem* widget2 =
                dynamic_cast<DGLBreakPointDialogItem*>(
                    m_Ui.rightListWidget->item(j));
            assert(widget2);
            if (widget1->get() == widget2->get()) {
                found = true;
            }
        }
        if (!found) {
            new DGLBreakPointDialogItem(widget1->get(), m_Ui.rightListWidget);
        }
    }
    m_Ui.leftListWidget->selectionModel()->clearSelection();
}

void DGLBreakPointDialog::deleteBreakPoint() {
    QList<QListWidgetItem*> list = m_Ui.rightListWidget->selectedItems();
    for (int i = 0; i < list.count(); i++) {
        delete list.at(i);
    }
}

void DGLBreakPointDialog::searchBreakPoint(const QString& prefix) {
    if (prefix.length()) {
        QItemSelection selection;

        bool first = true;

        for (int j = 0; j < m_Ui.leftListWidget->count(); j++) {
            QListWidgetItem* item = m_Ui.leftListWidget->item(j);
            if (item->text().startsWith(prefix)) {
                QModelIndex idx = m_Ui.leftListWidget->model()->index(j, 0);
                selection.select(idx, idx);

                if (first) {
                    first = false;
                    m_Ui.leftListWidget->scrollToItem(
                        item, QAbstractItemView::PositionAtTop);
                }
            }
        }
        m_Ui.leftListWidget->selectionModel()->select(
            selection, QItemSelectionModel::ClearAndSelect);
    } else {
        m_Ui.leftListWidget->selectionModel()->clearSelection();
    }
}
