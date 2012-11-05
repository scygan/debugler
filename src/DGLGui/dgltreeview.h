#ifndef DGLTREEVIEW_H
#define DGLTREEVIEW_H

#include <QDockWidget>
#include <QTreeWidget>


#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"




class DGLTreeView : public QDockWidget {
    Q_OBJECT

public:
    DGLTreeView(QWidget* parrent, DglController* controller);
    ~DGLTreeView();


public slots:
    void enable();
    void disable();
    void breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&);


private: 
    QTreeWidget m_TreeWidget;
    bool m_Enabled;   
};

#endif // DGLTREEVIEW_H