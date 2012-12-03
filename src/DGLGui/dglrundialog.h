#ifndef DGLRUNDIALOG_H
#define DGLRUNDIALOG_H

#include <QtGui/QMainWindow>

#include "ui_dglrundialog.h"

class DGLRunDialog : public QDialog {
    Q_OBJECT

public:
    DGLRunDialog();
    ~DGLRunDialog();
    std::string getExecutable();
    std::string getCommandLineArgs();
    std::string getPath();

private slots:
    void updatePath();

private:
    QIntValidator m_portValidator;
    Ui::DGLRunDialogClass m_ui;
};

#endif // DGLRUNDIALOG_H