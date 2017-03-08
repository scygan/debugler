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

#include "dglmainwindow.h"

#include "dglqtgui.h"
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QToolBar>
#include <QProgressDialog>
#include <QFileDialog>

#include "dglbreakpointdialog.h"
#include "dglconfigdialog.h"
#include "dgltraceview.h"
#include "dgltreeview.h"
#include "dgltextureview.h"
#include "dglbufferview.h"
#include "dglframebufferview.h"
#include "dglfboview.h"
#include "dglrenderbufferview.h"
#include "dglshaderview.h"
#include "dglprogramview.h"
#include "dglgpuview.h"
#include "dglbacktraceview.h"
#include "dglstateview.h"
#include "dgladbinterface.h"
#include "dglprepareandroidwizard.h"

#include <DGLCommon/os.h>
#include <DGLCommon/version.h>

#include <fstream>

/**
 * Macros for QSettings variable names
 */
#define STRINGIFY(X) #X
#define SLASHIFY(X, Y) STRINGIFY(X / Y)
#define DGL_SETTINGS(X) SLASHIFY(GUI, X)
#define DGL_GEOMETRY_SETTINGS DGL_SETTINGS(geometry)
#define DGL_WINDOW_STATE_SETTINGS DGL_SETTINGS(windowState)
#define DGL_ColorScheme_SETTINGS DGL_SETTINGS(colorScheme)
#define DGL_ADB_PATH_SETTINGS STRINGIFY(adbPath)

/**
 * Array of available main window color settings
 */
struct DGLColorScheme {
    const char *name; /**< Display name */
    const char *file; /**< Resource file name wyth styleshet */
} dglColorSchemes[DGLNUM_COLOR_SCHEMES] = {
          {"Default", ":/res/default.stylesheet"},
          {"Dark Orange", ":/res/darkorange.stylesheet"}, };

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define HICON_TO_QPIXMAP(hicon) QPixmap::fromHICON(hicon)
#else
// Private API, use QtWinExtras when it becomes available (oh my...)
QPixmap qt_pixmapFromWinHICON(HICON icon);
#define HICON_TO_QPIXMAP(hicon) qt_pixmapFromWinHICON(hicon)
#endif

DGLMainWindow::DGLMainWindow(QWidget *_parent, Qt::WindowFlags flags)
        : QMainWindow(_parent, flags), m_BusyDialog(this), m_ProjectSaved(false) {

#pragma warning(push)
#pragma warning(disable : 4127)    // conditional expression is constant
    Q_INIT_RESOURCE(dglmainwindow);
#pragma warning(pop)

    boost::shared_ptr<OsIcon> icon(Os::createIcon());
#ifdef _WIN32
    setWindowIcon(QIcon(HICON_TO_QPIXMAP((HICON)icon->get())));
#endif

    // load designer UI

    m_ui.setupUi(this);
    setDockNestingEnabled(true);

    // create all widgets, actions iteractions etc...

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();
    createInteractions();

    // read QSettings

    readSettings();

    showConfig();

    if (QCoreApplication::arguments().size() == 2) {
        openProjectFromFile(QCoreApplication::arguments()[1]);
    }
}

DGLMainWindow::~DGLMainWindow() {}

void DGLMainWindow::closeEvent(QCloseEvent *_event) {

    if (!closeProject()) {
        _event->ignore();
        return;
    }

    debugStop();

    // store QSettings

    QSettings settings(DGL_MANUFACTURER, DGL_PRODUCT);
    settings.setValue(DGL_GEOMETRY_SETTINGS, saveGeometry());
    settings.setValue(DGL_WINDOW_STATE_SETTINGS, saveState());
    settings.setValue(DGL_ColorScheme_SETTINGS, m_ColorScheme);
    settings.setValue(
            DGL_ADB_PATH_SETTINGS,
            QString::fromStdString(DGLAdbInterface::get()->getAdbPath())
                    .toUtf8());

    // Send event to parent class

    _event->accept();
}

void DGLMainWindow::createDockWindows() {
    // Create all dock windows.

    {
        QDockWidget *dock = new DGLTraceView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::BottomDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }
    {
        QDockWidget *dock = new DGLTreeView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }
    {
        QDockWidget *dock = new DGLStateView(this, &m_controller);
        dock->setMinimumSize(QSize(0, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::BottomDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }
    {
        QDockWidget *dock = new DGLGPUView(this, &m_controller);
        dock->setMinimumSize(QSize(0, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::BottomDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }
    QDockWidget *tabifyMaster;
    {
        QDockWidget *dock = new DGLTextureView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setMinimumSize(QSize(600, 0));
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyMaster = dock;
    }
    {
        QDockWidget *dock = new DGLBufferView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLFramebufferView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLFBOView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLRenderbufferView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLShaderView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLProgramView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
    }
    {
        QDockWidget *dock = new DGLBackTraceView(this, &m_controller);
        dock->setMinimumSize(QSize(0, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::RightDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
        tabifyDockWidget(tabifyMaster, dock);
        dock->hide();
    }
}

void DGLMainWindow::createMenus() {
    projectMenu = menuBar()->addMenu(tr("&Project"));
    projectMenu->addAction(newProjectAct);
    projectMenu->addAction(openProjectAct);
    projectMenu->addSeparator();
    projectMenu->addAction(projectProperiesAct);
    projectMenu->addAction(saveProjectAct);
    projectMenu->addAction(saveAsProjectAct);
    projectMenu->addSeparator();
    projectMenu->addAction(closeProjectAct);

    projectMenu->addSeparator();
    projectMenu->addAction(quitAct);

    debugMenu = menuBar()->addMenu(tr("&Debug"));
    
    debugMenu->addAction(debugStartAct);
    debugMenu->addAction(debugContinueAct);
    debugMenu->addAction(debugStopAct);
    debugMenu->addSeparator();
    debugMenu->addAction(debugTerminateAct);
    debugMenu->addSeparator();
    debugMenu->addAction(debugInterruptAct);
    debugMenu->addAction(debugStepAct);
    debugMenu->addAction(debugStepDrawCallAct);
    debugMenu->addAction(debugStepFrameAct);
    
    breakpointsMenu = menuBar()->addMenu(tr("&Breakpoints"));
    breakpointsMenu->addAction(addDeleteBreakPointsAct);
    breakpointsMenu->addSeparator();
    breakpointsMenu->addAction(setBreakOnGLErrorAct);
    breakpointsMenu->addAction(setBreakOnDebugOutputAct);
    breakpointsMenu->addAction(setBreakOnCompilerErrAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addSeparator();
    ColorSchemeMenu = viewMenu->addMenu(tr("Color Schemes"));
    for (uint i = 0; i < DGLNUM_COLOR_SCHEMES; i++) {
        ColorSchemeMenu->addAction(setColorSchemeActs[i]);
    }

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(configurationAct);

    menuBar()->addSeparator();
    toolsMenu->addAction(prepareAndroidAct);
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void DGLMainWindow::createToolBars() {
    debugToolBar = addToolBar(tr("Debug"));
    debugToolBar->setObjectName("DGLToolbar");
    debugToolBar->addAction(debugStartAct);
    debugToolBar->addAction(debugContinueAct);
    debugToolBar->addAction(debugInterruptAct);
    debugToolBar->addAction(debugStepAct);
    debugToolBar->addAction(debugStepDrawCallAct);
    debugToolBar->addAction(debugStepFrameAct);
    debugToolBar->addAction(debugStopAct);
    debugToolBar->addAction(addDeleteBreakPointsAct);
    debugToolBar->addSeparator();
    debugToolBar->addAction(addDeleteBreakPointsAct);
    debugToolBar->addAction(setBreakOnGLErrorAct);
    debugToolBar->addAction(setBreakOnDebugOutputAct);
    debugToolBar->addAction(setBreakOnCompilerErrAct);
}

void DGLMainWindow::createStatusBar() {

    // set initial status

    statusBar()->showMessage(tr("Ready"));
}

void DGLMainWindow::createActions() {

    // create "QActions" - bindings between mainwindow clickable widgets,
    // and
    // local slots

    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));
    CONNASSERT(quitAct, SIGNAL(triggered()), this, SLOT(close()));
    quitAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    CONNASSERT(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    aboutAct->setShortcut(QKeySequence(Qt::Key_F1));

    newProjectAct = new QAction(tr("&New Project..."), this);
    newProjectAct->setStatusTip(tr("Created new debugging project"));
    CONNASSERT(newProjectAct, SIGNAL(triggered()), this, SLOT(newProject()));
    newProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));


    openProjectAct = new QAction(tr("&Open Project..."), this);
    openProjectAct->setStatusTip(tr("Opens a debugging project"));
    CONNASSERT(openProjectAct, SIGNAL(triggered()), this, SLOT(openProject())); //TODO: implement
    openProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));

    saveProjectAct = new QAction(tr("&Save Project"), this);
    saveProjectAct->setStatusTip(tr("Save a debugging project"));
    CONNASSERT(saveProjectAct, SIGNAL(triggered()), this, SLOT(saveProject())); //TODO: implement
    saveProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    saveAsProjectAct = new QAction(tr("&Save Project As ..."), this);
    saveAsProjectAct->setStatusTip(tr("Save a debugging project as as a file ..."));
    CONNASSERT(saveAsProjectAct, SIGNAL(triggered()), this, SLOT(saveProjectAs())); //TODO: implement
    saveAsProjectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
    

    projectProperiesAct = new QAction(tr("&Project properties..."), this);
    projectProperiesAct->setStatusTip(tr("Change properties of current project"));
    CONNASSERT(projectProperiesAct, SIGNAL(triggered()), this, SLOT(projectProperties()));
    

    closeProjectAct = new QAction(tr("&Close project"), this);
    closeProjectAct->setStatusTip(tr("Created new debugging project"));
    CONNASSERT(closeProjectAct, SIGNAL(triggered()), this, SLOT(closeProject()));

    debugStartAct = new QAction(tr("&Start debugging"), this);
    debugStartAct->setStatusTip(tr("Stop debugging."));
    CONNASSERT(debugStartAct, SIGNAL(triggered()), this, SLOT(debugStart()));
    CONNASSERT(&m_controller, SIGNAL(setDisconnected(bool)), debugStartAct,
        SLOT(setVisible(bool)));
    debugStartAct->setShortcut(QKeySequence(Qt::Key_F5));

    debugStopAct = new QAction(tr("Sto&p debugging"), this);
    debugStopAct->setStatusTip(tr("Stop debugging."));
    CONNASSERT(debugStopAct, SIGNAL(triggered()), this, SLOT(debugStop()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugStopAct,
        SLOT(setEnabled(bool)));
    debugStopAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F5));
    debugStopAct->setEnabled(false);

    debugTerminateAct = new QAction(tr("Terminate"), this);
    debugTerminateAct->setStatusTip(tr("Terminate debugged process."));
    CONNASSERT(debugTerminateAct, SIGNAL(triggered()), this, SLOT(debugTerminate()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugTerminateAct,
        SLOT(setEnabled(bool)));
    debugTerminateAct->setEnabled(false);

    debugContinueAct = new QAction(tr("&Continue"), this);
    debugContinueAct->setStatusTip(tr("Continue program execution"));
    CONNASSERT(debugContinueAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugContinue()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugContinueAct,
               SLOT(setVisible(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugContinueAct,
               SLOT(setEnabled(bool)));
    debugContinueAct->setShortcut(QKeySequence(Qt::Key_F5));
    debugContinueAct->setVisible(false);

    debugInterruptAct = new QAction(tr("&Break on next call"), this);
    debugInterruptAct->setStatusTip(
            tr("Break program execution on GL call"));
    CONNASSERT(debugInterruptAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugInterrupt()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugInterruptAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setRunning(bool)), debugInterruptAct,
               SLOT(setEnabled(bool)));
    debugInterruptAct->setShortcut(QKeySequence(Qt::Key_F6));
    debugInterruptAct->setEnabled(false);

    debugStepAct = new QAction(tr("&Step"), this);
    debugStepAct->setStatusTip(tr("Step one GL call"));
    CONNASSERT(debugStepAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugStep()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugStepAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugStepAct,
               SLOT(setEnabled(bool)));
    debugStepAct->setShortcut(QKeySequence(Qt::Key_F11));
    debugStepAct->setEnabled(false);

    debugStepDrawCallAct = new QAction(tr("&Draw step"), this);
    debugStepDrawCallAct->setStatusTip(tr("Step one GL drawing call"));
    CONNASSERT(debugStepDrawCallAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugStepDrawCall()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugStepDrawCallAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugStepDrawCallAct,
               SLOT(setEnabled(bool)));
    debugStepDrawCallAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F11));
    debugStepDrawCallAct->setEnabled(false);

    debugStepFrameAct = new QAction(tr("&Frame step"), this);
    debugStepFrameAct->setStatusTip(tr("Step one GL frame"));
    CONNASSERT(debugStepFrameAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugStepFrame()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugStepFrameAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugStepFrameAct,
               SLOT(setEnabled(bool)));
    debugStepFrameAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F11));
    debugStepFrameAct->setEnabled(false);

    addDeleteBreakPointsAct = new QAction(tr("&Breakpoints..."), this);
    addDeleteBreakPointsAct->setStatusTip(tr("Add or remove breakpoints"));
    CONNASSERT(addDeleteBreakPointsAct, SIGNAL(triggered()), this,
               SLOT(addDeleteBreakPoints()));
    addDeleteBreakPointsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));

    setBreakOnGLErrorAct = new QAction(tr("Break on GL error"), this);
    setBreakOnGLErrorAct->setStatusTip(
            tr("Break execution on GL error (glGetError() != GL_NO_ERROR)"));

    // this action has a state - it is checbox-like checkable

    DGLConfiguration& currentConfig = m_controller.getConfig();

    setBreakOnGLErrorAct->setCheckable(true);
    setBreakOnGLErrorAct->setChecked(currentConfig.m_BreakOnGLError);
    CONNASSERT(setBreakOnGLErrorAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    setBreakOnDebugOutputAct = new QAction(tr("Break on debug output"), this);
    setBreakOnDebugOutputAct->setStatusTip(
            tr("Break execution on debug output message"));

    // this action has a state - it is checbox-like checkable

    setBreakOnDebugOutputAct->setCheckable(true);
    setBreakOnDebugOutputAct->setChecked(
            currentConfig.m_BreakOnGLError);
    CONNASSERT(setBreakOnDebugOutputAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    setBreakOnCompilerErrAct =
            new QAction(tr("Break on compiler/linker error"), this);
    setBreakOnCompilerErrAct->setStatusTip(
            tr("Break execution on debug GLSL compiler or linker error"));

    // this action has a state - it is checbox-like checkable

    setBreakOnCompilerErrAct->setCheckable(true);
    setBreakOnCompilerErrAct->setChecked(
            currentConfig.m_BreakOnCompilerError);
    CONNASSERT(setBreakOnCompilerErrAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    // Only one color scheme can be choosed - put all related actions to
    // action
    // group

    setColorSchemeActGroup = new QActionGroup(this);

    // iterate through all color schemes. for each one create one action

    for (uint i = 0; i < DGLNUM_COLOR_SCHEMES; i++) {
        setColorSchemeActs[i] = new QAction(tr(dglColorSchemes[i].name), this);
        setColorSchemeActs[i]->setCheckable(true);
        setColorSchemeActs[i]->setActionGroup(setColorSchemeActGroup);
        setColorSchemeActs[i]->setStatusTip(tr("Set this color scheme"));

        // connect all color scheme actions to one mapper, so we can connect
        // it
        // later to only one signal

        m_SetColorSchemeSignalMapper.setMapping(setColorSchemeActs[i], i);
        CONNASSERT(setColorSchemeActs[i], SIGNAL(triggered()),
                   &m_SetColorSchemeSignalMapper, SLOT(map()));
    }

    // mapper maps connected actions to one emitted signal by int parameter.
    // Connect this signal to "this"

    CONNASSERT(&m_SetColorSchemeSignalMapper, SIGNAL(mapped(int)), this,
               SLOT(setColorScheme(int)));

    configurationAct = new QAction(tr("Configuration..."), this);
    configurationAct->setStatusTip(tr("Configuration options"));
    CONNASSERT(configurationAct, SIGNAL(triggered()), this, SLOT(configure()));

    prepareAndroidAct = new QAction(tr("Prepare Android device..."), this);
    prepareAndroidAct->setStatusTip(tr("Installs " DGL_PRODUCT " on Android device"));
    CONNASSERT(prepareAndroidAct, SIGNAL(triggered()), this,
               SLOT(androidPrepare()));
}

void DGLMainWindow::createInteractions() {

    // connect some signals from DGLcontroller to UI

    CONNASSERT(&m_controller, SIGNAL(newStatus(const QString &)),
               m_ui.statusBar, SLOT(showMessage(const QString &)));
    CONNASSERT(&m_controller,
               SIGNAL(connectionLost(const QString &, const QString &)), this,
               SLOT(connectionLost(const QString &, const QString &)));
    CONNASSERT(&m_controller, SIGNAL(debugeeInfo(const std::string &)), this,
               SLOT(updateWindowCaption(const std::string &)));

    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), this,
        SLOT(onConnect(bool)));

    CONNASSERT(&m_controller, SIGNAL(breaked(const CalledEntryPoint, uint)), this,
        SLOT(bringupToFront()));

    CONNASSERT(&m_BusyDialog, SIGNAL(canceled()), this,
        SLOT(debugStop()));
}

void DGLMainWindow::readSettings() {

    // read settings

    QSettings settings(DGL_MANUFACTURER, DGL_PRODUCT);
    restoreGeometry(settings.value(DGL_GEOMETRY_SETTINGS).toByteArray());
    restoreState(settings.value(DGL_WINDOW_STATE_SETTINGS).toByteArray());

    DGLAdbInterface::get()->setAdbCookieFactory(
        std::make_shared<DGLAdbCookieFactory>(
            settings.value(DGL_ADB_PATH_SETTINGS).toString().toStdString()));

    // decode and set actual color scheme from settings

    uint ColorScheme = settings.value(DGL_ColorScheme_SETTINGS).toUInt();
    setColorScheme(ColorScheme);
}

void DGLMainWindow::setColorScheme(int colorScheme) {

    // check if scheme id is valid

    if (colorScheme >= 0 && colorScheme < DGLNUM_COLOR_SCHEMES) {

        // check if apropriate action is already checked, if not check it

        if (!setColorSchemeActs[colorScheme]->isChecked())
            setColorSchemeActs[colorScheme]->setChecked(true);

        // store color scheme id

        m_ColorScheme = colorScheme;

        // load color scheme from resource file

        QString fileName(dglColorSchemes[colorScheme].file);
        QFile file(fileName);
        if (file.open(QFile::ReadOnly)) {

            // if file exists, read it and set QApplication style

            QString colorSchemeSheet = QLatin1String(file.readAll());
            qApp->setStyleSheet(colorSchemeSheet);
        }
    }
}

void DGLMainWindow::bringupToFront() {
    raise();
    activateWindow();
}

bool DGLMainWindow::haveProject() {
    if (!m_project) {
        if (QMessageBox::question(this, tr("No project is opened."), tr("No project is opened - create new?"), QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
            newProject();
        }
        //we always return false here: we fool user into creating new project, 
        //so he propably won't remember what he clicked earlier.
        return false;
    }
    return true;
}

void DGLMainWindow::configure() {
    DGLConfigDialog dialog(m_controller.getConfig(),
                           DGLAdbInterface::get()->getAdbPath());
    if (dialog.exec() == QDialog::Accepted) {
        m_controller.sendConfig(dialog.getConfig());
        DGLAdbInterface::get()->setAdbCookieFactory(
            std::make_shared<DGLAdbCookieFactory>(dialog.getAdbPath().toStdString()));
    }
}

void DGLMainWindow::androidPrepare() {
    dglPrepareAndroidWizard::Wizard wizard;
    wizard.exec();
}

void DGLMainWindow::updateWindowCaption(const std::string &processName) {

    if (!m_project) {
        setWindowTitle("No project opened - " DGL_PRODUCT);
    } else {

        QString projectName = "Untitled project";
        if (m_SavedProjectPath.size()) {
            projectName = m_SavedProjectPath;
        }

        if (processName.length()) {
            setWindowTitle(projectName + " ( " + QString::fromStdString(processName) + " ) - " DGL_PRODUCT);
        } else {
            setWindowTitle(projectName + " ( disconnected ) - " DGL_PRODUCT);
        }
    }
}

void DGLMainWindow::about() {
    QMessageBox::about(
            this, tr("About " DGL_PRODUCT),
            QString("<b>" DGL_PRODUCT) + " " + QString::fromStdString(getVersion()) + 
               tr("</b>, The OpenGL(R) debugger<br/><br/>"
               "Copyright (C) 2013 " DGL_MANUFACTURER ".<br/><br/> "
               "<a href=\"https://github.com/debugler/debugler\"/>"));
}

void DGLMainWindow::newProject() {

    if (m_project) {
        if (!closeProject()) {
            return;
        }
    }

    if (m_ProjectDialog.exec() == QDialog::Accepted) {

        // if dialog is successfull, get project and start debug
        std::shared_ptr<DGLProject> project = m_ProjectDialog.getProject();
        
        setCurrentProject(project);

        if (project) {

            m_ProjectSaved = false;

            debugStart();
        }
    }
}

bool DGLMainWindow::closeProject() {
    
    if (m_controller.isConnected()) {
        if ( QMessageBox::question(
            this, "Confirm close",
            "Debugging session is in progress. Close project?",
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
                return false;
        } else {
            debugStop();
        }
    }

    updateWindowCaption("");

    if (m_project && !m_ProjectSaved) {
        
        auto messageBoxResult = QMessageBox::question(
            this, "Save project",
            "Do you want to save changes to current project?",
            QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        
        switch (messageBoxResult) {
            case QMessageBox::Yes:
                if (!saveProject()) {
                    return false;
                }
                break;

            case QMessageBox::No:
                break;

            case QMessageBox::Cancel:
                // return false, so closing is interrupted
                return false;

            default:
                DGL_ASSERT(!"Unrecognized message box result");
        }
    }

    setCurrentProject(std::shared_ptr<DGLProject>());

    m_ProjectSaved = false;

    return true;
}

void DGLMainWindow::projectProperties() {
    if (haveProject()) {
        m_ProjectDialog.loadPropertiesFromProject(m_project.get());
        if (m_ProjectDialog.exec() == QDialog::Accepted) {

            std::shared_ptr<DGLProject> newCurrentProject =  m_ProjectDialog.getProject();

            m_ProjectSaved = ( !newCurrentProject || *newCurrentProject == *m_project );

            setCurrentProject(newCurrentProject);
        }
    }
}

void DGLMainWindow::openProject() {

    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open project..."), QString(), tr("Debugler project files (*.dglproj)"));

    if (filePath.isEmpty()) {
        return;
    }

    openProjectFromFile(filePath);
}


bool DGLMainWindow::saveProject() {
    
    if (m_SavedProjectPath.size() == 0) {
        return saveProjectAs();
    }

    return saveProjectToFile(m_SavedProjectPath);
}

bool DGLMainWindow::saveProjectAs() {

    QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save project as..."), QString(), tr("Debugler project files (*.dglproj)"));

    if (filePath.isEmpty()) {
        return false;
    }

    return saveProjectToFile(filePath);
}

bool DGLMainWindow::saveProjectToFile(QString filePath) {
    
    if (!m_project) {
        return true;
    }

    try {

        std::ofstream outputStream;

        outputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        outputStream.open(filePath.toStdString());
        m_project->saveToStream(outputStream);

        m_ProjectSaved = true;

        m_SavedProjectPath = filePath;

        updateWindowCaption("");

        return true;

    } catch (const std::ifstream::failure& err) {

        QMessageBox::critical(NULL, tr("Cannot write file"),
            QString::fromStdString(err.what()));

    } catch (...) {
        QMessageBox::critical(NULL, tr("Cannot write file"),
            tr("Unknown exception occurred while writing the file"));
    }
    return false;   
}

void DGLMainWindow::openProjectFromFile(QString filePath) {
    try {

        if (m_project) {
            if (!closeProject()) {
                return;
            }
        }

        std::ifstream inputStream;

        inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        inputStream.open(filePath.toStdString());

        std::shared_ptr<DGLProject> project = DGLProject::createFromStream(inputStream);

        if (!project) {
            throw std::runtime_error("File does not contain a Debugler project");
        }

        m_SavedProjectPath = filePath;

        setCurrentProject(project);

        m_ProjectSaved = true;

        debugStart();

    } catch (const std::ifstream::failure& err) {

        QMessageBox::critical(NULL, tr("Cannot read from file"),
            QString::fromStdString(err.what()));

    } catch (...) {
        QMessageBox::critical(NULL, tr("Cannot read from file"),
            tr("Unknown exception occurred while reading the file"));
    }
}

void DGLMainWindow::setCurrentProject(std::shared_ptr<DGLProject> project) {

    m_project = project;

    if (project) {
        CONNASSERT(m_project.get(), SIGNAL(debugStarted(std::string, std::string)), this, SLOT(onDebugStartedConnectReady(std::string, std::string)));
        CONNASSERT(m_project.get(), SIGNAL(debugError(QString, QString)), this, SLOT(onDebugError(QString, QString)));
        CONNASSERT(m_project.get(), SIGNAL(debugExit(QString)), this, SLOT(onDebugExit(QString)));
    }

    updateWindowCaption("");
}

void DGLMainWindow::debugStart() {
    
    debugStop();

    if (haveProject()) {

        try {
            m_BusyDialog.show();
            m_project->startDebugging();
        }
        catch (const std::runtime_error &err) {
            m_BusyDialog.hide();
            QMessageBox::critical(NULL, tr("Fatal Error"),
                QString::fromStdString(err.what()));
        }
    }
}

void DGLMainWindow::onDebugStartedConnectReady(const std::string& address, std::string port) {
    m_controller.connectServer(address, port);
}

void DGLMainWindow::onDebugError(QString error, QString message) {
    debugStop();
    QMessageBox::critical(this, error, message);
}

void DGLMainWindow::onDebugExit(QString reason) {
    debugStop();
    QMessageBox::information(this, tr("Debugging stopped"), reason);
}

void DGLMainWindow::debugStop() {

    if (m_controller.isConnected() && m_project->shouldTerminateOnStop()) {
        m_controller.debugTerminate();
    }

    m_controller.disconnectServer();
    if (m_project) {
        m_project->stopDebugging();
    }
    m_BusyDialog.hide();
}

void DGLMainWindow::debugTerminate() {
     if (m_controller.isConnected()) {
         m_controller.debugTerminate();
     }
}

void DGLMainWindow::addDeleteBreakPoints() {

    // execute break point dialog. Already set breakpoint are
    // sourced from provided DGLController object

    DGLBreakPointDialog dialog(&m_controller);
    if (dialog.exec() == QDialog::Accepted) {

        // if dialog was successfull retrieve list bkpoints and feed them
        // to breakpoint controller in DGLcontroller

        m_controller.getBreakPoints()->setCurrent(dialog.getBreakPoints());
    }
}

void DGLMainWindow::setBreakOnWhatever(bool) {

    // This action enables breaking on various events, like GL error or debug
    // output
    // tell DGLController to configure it's debugee

    DGLConfiguration& currentConfig = m_controller.getConfig();

    currentConfig.m_BreakOnGLError =
            setBreakOnGLErrorAct->isChecked();
    currentConfig.m_BreakOnDebugOutput =
            setBreakOnDebugOutputAct->isChecked();
    currentConfig.m_BreakOnCompilerError =
            setBreakOnCompilerErrAct->isChecked();
    m_controller.sendConfig();
    showConfig();
}

void DGLMainWindow::onConnect(bool connected) {
    if (connected) {
        m_BusyDialog.hide();
    }
}

void DGLMainWindow::connectionLost(const QString &title, const QString &msg) {
    if (m_controller.isConnected()) {
        // we still may have a process, that should terminate now

        // m_process has still handler slots connected here, so user
        // will be informed about process exit.
        m_project->stopDebugging();
    } else {
        debugStop();
        QMessageBox::critical(this, title, msg);
    }
}

void DGLMainWindow::showConfig() {
    
    DGLConfiguration& currentConfig = m_controller.getConfig();

    setBreakOnGLErrorAct->setChecked(currentConfig.m_BreakOnGLError);
    setBreakOnDebugOutputAct->setChecked(
            currentConfig.m_BreakOnDebugOutput);
    setBreakOnCompilerErrAct->setChecked(
            currentConfig.m_BreakOnCompilerError);
}
