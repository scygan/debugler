#ifndef DGLGPUVIEW_H
#define DGLGPUVIEW_H

#include <QDockWidget>


#include "dglcontroller.h"
#include "ui_dglgpuview.h"


class DGLGPUView : public QDockWidget {
    Q_OBJECT

public:
    DGLGPUView(QWidget* parrent, DglController* controller);

public slots:
    void setConnected(bool);
    void update(const DGLResource&);
    void error(const std::string&);

private: 
    DGLResourceListener * m_Listener;
    DglController* m_Controller;
    Ui_DGLGPUView m_Ui;
};

#endif // DGLTREEVIEW_H