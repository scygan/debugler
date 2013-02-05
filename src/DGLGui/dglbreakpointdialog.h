#ifndef DGLBREAKPOINTDIALOG_H
#define DGLBREAKPOINTDIALOG_H

#include <QtGui/QMainWindow>

#include "ui_dglbreakpointdialog.h"
#include "dglcontroller.h"

class DGLBreakPointDialog : public QDialog {
    Q_OBJECT

public:
    DGLBreakPointDialog(DglController * controller);
    ~DGLBreakPointDialog();

    std::set<Entrypoint> getBreakPoints();

public slots:
    void addBreakPoint();
    void deleteBreakPoint();
    void searchBreakPoint(const QString&);

private:
    Ui_BreakPointDialog m_Ui;
    DglController* m_Controller;
};

#endif // DGLCONNECTDIALOG_H