/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef DGLMAINWINDOW_H
#define DGLMAINWINDOW_H

#include "dglqtgui.h"
#include <QMainWindow>
#include <QListWidget>
#include <QSignalMapper>
#include <QProgressDialog>

#include "ui_dglmainwindow.h"

#include "dglcontroller.h"
#include "dglconnectandroiddialog.h"

#include "dglprojectdialog.h"
#include "dglproject_base.h"
#include "dglbusydialog.h"


/**
 * Number of avaliable color schemes
 */
#define DGLNUM_COLOR_SCHEMES 2    // TODO: move this, or make more flexible

/**
 * Main window class
 */
class DGLMainWindow : public QMainWindow {
    Q_OBJECT

   public:
    /**
     * Ctor, called from bootstrap in main()
     */
    DGLMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~DGLMainWindow();

   private
slots:
    /**
     * Slot displaying about window
     */
    void about();

    /**
     * Slot for connection status called from dglcontroller
     */
    void onConnect(bool connected);

    /**
     * Slot for connection lost, called from dglcontroller
     * @param Error title
     * @param Error message
     */
    void connectionLost(const QString &, const QString &);

    /**
     * Slot displaying "New Project dialog window..."
     */
    void newProject();

    /**
     * Slot for closing current project
     */
    void closeProject();

     /**
     * Slot for displaying project properties dialog
     */
    void projectProperties();

    /**
     * Slot for displaing "Attach to Android App window..."
     */
    void attachAndroidApp();

    /**
     * Slot for starting debugging session
     */
    void debugStart();

    /**
     * Slot called on successful debug startup in project.
     *
     * Signalizes readiness to perform connection.
     */
    void onDebugStartedConnectReady(std::string address, std::string port);

    /** 
     * Slot called on async error from debug startup in project
     */
    void onDebugError(QString error, QString message);

    /** 
     * Slot called on async debug exit from project
     */
    void onDebugExit(QString reason);

    /**
     * Slot for terminating current debugging session
     */
    void debugStop();

    /**
     * Slot for displaying bkpoints window
     */
    void addDeleteBreakPoints();

    /**
     * Slot for manipulating "Break on GL error" and "Break on debug output"
     * setting
     */
    void setBreakOnWhatever(bool);

    /**
     * Shared slot for all color scheme-change actions.
     * This actually makes color scheme change
     * @param ColorScheme id of color scheme
     */
    void setColorScheme(int ColorScheme);

    /**
     * Slot for displaying configuration window
     */
    void configure();

    /**
     * Slot for running Android prepareAndroid wizard
     */
    void androidPrepare();

    /**
     * Called by dgl controller to give debugee process info. Used to populate
     * window caption
     */
    void debugeeInfo(const std::string &);

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
     * Private method for checking if project is opened
     */
    bool haveProject();

    /**
      * Method called to read all QSettings and fed them
      * to proper objects
      */
    void readSettings();

    /**
     * Synchronize widgets with current debugee configuration in controller
     */
    void showConfig();

    /**
      * Method intercepring main window close event
      */
    virtual void closeEvent(QCloseEvent *event);

    // menus

    QMenu *projectMenu;
    QMenu *debugMenu;
    QMenu *breakpointsMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *ColorSchemeMenu;
    QMenu *helpMenu;

    // toolbars

    QToolBar *debugToolBar;

    // actions

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;

    QAction *newProjectAct;
    QAction *projectProperiesAct;
    QAction *openProjectAct;
    QAction *saveProjectAct;
    QAction *saveAsProjectAct;
    QAction *closeProjectAct;

    QAction *attachAndroidAct; //TODO: delete

    QAction *debugStartAct;
    QAction *debugStopAct;
    QAction *debugInterruptAct;
    QAction *debugContinueAct;
    QAction *debugStepAct;
    QAction *debugStepFrameAct;
    QAction *debugStepDrawCallAct;
    QAction *addDeleteBreakPointsAct;
    QAction *setBreakOnGLErrorAct;
    QAction *setBreakOnDebugOutputAct;
    QAction *setBreakOnCompilerErrAct;
    QAction *setColorSchemeActs[DGLNUM_COLOR_SCHEMES];
    QAction *configurationAct;
    QAction *prepareAndroidAct;
    /**
     * Action group for all actions from setColorSchemeActs[]
     */
    QActionGroup *setColorSchemeActGroup;

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

    std::shared_ptr<DGLProject> m_project;


    DGLProjectDialog m_ProjectDialog;

    DGLConnectAndroidDialog m_ConnectAndroidDialog;

    /**
     * Process starting busy progress dialog
     */
    DGLbusyDialog m_BusyDialog;
};

#endif    // DGLMAINWINDOW_H
