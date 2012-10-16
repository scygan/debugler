#ifndef DGLCONNECTDIALOG_H
#define DGLCONNECTDIALOG_H

#include <QtGui/QMainWindow>
 #include <QIntValidator>

#include "ui_dglconnectdialog.h"

class DGLConnectDialog : public QDialog
{
    Q_OBJECT

public:
    DGLConnectDialog();
    ~DGLConnectDialog();
    std::string getAddress();
    std::string getPort();

private:
    QIntValidator m_portValidator;
    Ui::DGLConnectDialogClass m_ui;
};

#endif // DGLCONNECTDIALOG_H