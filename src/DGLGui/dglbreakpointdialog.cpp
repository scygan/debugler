
#include "dglbreakpointdialog.h"
#include <dglgui.h>

class DGLBreakPointDialogItem: public QListWidgetItem {
public:
    DGLBreakPointDialogItem(Entrypoint e, QListWidget* parrent):QListWidgetItem(QString(GetEntryPointName(e)), parrent),m_Entrypoint(e) {}
    Entrypoint get() { return m_Entrypoint; }
private:
    Entrypoint m_Entrypoint;
};

DGLBreakPointDialog::DGLBreakPointDialog(DglController * controller):m_Controller(controller) {
    m_Ui.setupUi(this);
    m_Ui.leftListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    m_Ui.rightListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    for (Entrypoint i = 0; i < NUM_ENTRYPOINTS; i++) {
        new DGLBreakPointDialogItem(i, m_Ui.leftListWidget);
    }

    std::set<Entrypoint> currentBreakPoints = m_Controller->getBreakPoints()->getCurrent();
    for (std::set<Entrypoint>::iterator i = currentBreakPoints.begin(); i != currentBreakPoints.end(); i++) {
        new DGLBreakPointDialogItem(*i, m_Ui.rightListWidget);
    }

    CONNASSERT(connect(m_Ui.addButton, SIGNAL(clicked()), this, SLOT(addBreakPoint())));
    CONNASSERT(connect(m_Ui.deleteButton, SIGNAL(clicked()), this, SLOT(deleteBreakPoint())));
}

DGLBreakPointDialog::~DGLBreakPointDialog() {}

std::set<Entrypoint> DGLBreakPointDialog::getBreakPoints() {
    std::set<Entrypoint> ret;
    for (uint i = 0; i < m_Ui.rightListWidget->count(); i++) {
        DGLBreakPointDialogItem* widget = dynamic_cast<DGLBreakPointDialogItem*>( m_Ui.rightListWidget->item(i));
        assert(widget);
        ret.insert(widget->get());
    }
    return ret;
}

void DGLBreakPointDialog::addBreakPoint() {
    QList<QListWidgetItem*> list = m_Ui.leftListWidget->selectedItems();

    for (uint i = 0; i < list.count(); i++) {
        DGLBreakPointDialogItem* widget1 = dynamic_cast<DGLBreakPointDialogItem*>(list.at(i));
        assert(widget1);
        bool found = false;
        for (uint j = 0; j < m_Ui.rightListWidget->count(); j++) {
            DGLBreakPointDialogItem* widget2 = dynamic_cast<DGLBreakPointDialogItem*>( m_Ui.rightListWidget->item(j));
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
    for (uint i = 0; i < list.count(); i++) {
        delete list.at(i);
    }
}