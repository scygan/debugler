#ifndef DGLMAINWINDOW_H
#define DGLMAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QListWIdget>
#include <QSignalMapper>

#include "ui_dglmainwindow.h"

#include "dglcontroller.h"

/**
 * Number of avaliable color schemes
 */
#define DGLNUM_COLOR_SCHEMES 2  //TODO: move this, or make more flexible

/** 
 * Main window class
 */
class DGLMainWindow : public QMainWindow {
    Q_OBJECT

public:
    /** 
     * Ctor, called from bootstrap in main()
     */
    DGLMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    virtual ~DGLMainWindow();


private slots:
    /** 
     * Slot displaying about window
     */
    void about();

    /** 
     * Slot displaying error message
     * @param Error title
     * @param Error message
     */
    void errorMessage(const QString&, const QString&);

    /**
     * Slot for displaing "Attach to process window..." 
     */
    void attach();

    /**
     * Slot for disconnecting current connection
     */
    void disconnect();

    /** 
     * Slot for displaying bkpoints window
     */
    void addDeleteBreakPoints();

    /** 
     * Slot for manipulating "Break on GL error" setting"
    void setBreakOnGLError(bool);

    /**
     * Shared slot for all color scheme-change actions.
     * This actually makes color scheme change
     * @param ColorScheme id of color scheme
     */
    void setColorScheme(int ColorScheme);

private:

    /**
     * Private funcion for creation & initialization of all QActons
     */
    void createActions();

    /**
     *Private Method for creation main menus
     */
    void createMenus();

    /**
     * Private Method for creation of all toolbars
     */
    void createToolBars();

    void createStatusBar();
    void createDockWindows();
    void createInteractions();

    /** 
      * Method called to read all QSettings and fed them 
      * to proper objects
      */
    void readSettings();

    /**
      * Method intercepring main window close event
      */
    virtual void closeEvent(QCloseEvent *event);

    // menus

    QMenu *fileMenu;
    QMenu *debugMenu;
    QMenu *viewMenu;
    QMenu *ColorSchemeMenu;
    QMenu *helpMenu;

    //toolbars

    QToolBar* debugToolBar;

    //actions

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
    QAction *setBreakOnGLErrorAct;
    QAction *setColorSchemeActs[DGLNUM_COLOR_SCHEMES];
    /**
     * Action group for all actions from setColorSchemeActs[]
     */
    QActionGroup * setColorSchemeActGroup;

    /**
     * Signal mapper integrating all signals from setColorSchemeActs[]
     */
    QSignalMapper m_SetColorSchemeSignalMapper;

    /**
     * Designer GUI member
     */
    Ui::DGLMainWindowClass m_ui;

    /**
     * Actual color schemd
     */
    int m_ColorScheme;

    /**
     * The DGLController object - the one and only interface to debugee from UI
     */
    DglController m_controller;
};

#endif // DGLMAINWINDOW_H
