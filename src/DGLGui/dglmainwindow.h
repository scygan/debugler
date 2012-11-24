#ifndef DGLMAINWINDOW_H
#define DGLMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QListWIdget>
#include <QSignalMapper>

#include "ui_dglmainwindow.h"

#include "dglcontroller.h"

#define DGLNUM_COLOR_SCHEMES 2

class DGLMainWindow : public QMainWindow {
    Q_OBJECT

public:
    DGLMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~DGLMainWindow();


private slots:
    void about();
    void errorMessage(const QString&, const QString&);

    void attach();
    void disconnect();
    void addDeleteBreakPoints();

    void setColorScheme(int ColorScheme);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void createInteractions();
    void readSettings();

    void closeEvent(QCloseEvent *event);

    QMenu *fileMenu;
    QMenu *debugMenu;
    QMenu *viewMenu;
    QMenu *ColorSchemeMenu;
    QMenu *helpMenu;

    QToolBar* debugToolBar;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;

    QAction *attachAct;
    QAction *disconnectAct;
    QAction *debugInterruptAct;
    QAction *debugContinueAct;
    QAction *debugStepAct;
    QAction *debugStepFrameAct;
    QAction *debugStepDrawCallAct;
    QAction *addDeleteBreakPointsAct;
    QAction *setColorSchemeActs[DGLNUM_COLOR_SCHEMES];
    QSignalMapper m_SetColorSchemeSignalMapper;

    Ui::DGLMainWindowClass m_ui;
    int m_ColorScheme;

    DglController m_controller;


};

#endif // DGLMAINWINDOW_H
