#ifndef DGLMAINWINDOW_H
#define DGLMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QListWIdget>

#include "ui_dglmainwindow.h"

#include "dglcontroller.h"

class DGLMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    DGLMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~DGLMainWindow();


private slots:
    void about();
    void errorMessage(const QString&, const QString&);

    void attach();



private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void createInteractions();
    
    QMenu *fileMenu;
    QMenu *debugMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;

    QAction *attachAct;
    QAction *debugStepAct;

    Ui::DGLMainWindowClass m_ui;

    DglController m_controller;


};

#endif // DGLMAINWINDOW_H
