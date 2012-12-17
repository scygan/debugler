#ifndef DGLSTATEVIEW_H
#define DGLSTATEVIEW_H

#include <QDockWidget>


#include "dglcontroller.h"
#include "ui_dglstateview.h"


class DGLStateView : public QDockWidget {
    Q_OBJECT

public:
    DGLStateView(QWidget* parrent, DglController* controller);

public slots:
    void setConnected(bool);
    void update(const DGLResource&);
    void error(const std::string&);

private: 
    DGLResourceListener * m_Listener;
    DglController* m_Controller;
    Ui::DGLStateView* m_Ui;
};

#endif // DGLTREEVIEW_H