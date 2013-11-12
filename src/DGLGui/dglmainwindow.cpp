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

#include "dglbreakpointdialog.h"
#include "dglconfigdialog.h"
#include "dgltraceview.h"
#include "dgltreeview.h"
#include "dgltextureview.h"
#include "dglbufferview.h"
#include "dglframebufferview.h"
#include "dglfboview.h"
#include "dglshaderview.h"
#include "dglprogramview.h"
#include "dglgpuview.h"
#include "dglstateview.h"
#include "dgladbinterface.h"

#include <DGLCommon/os.h>

/**
 * Macros indentifying entity, that stores & loads QSettings
 */
#define DGL_COMPANY "SaCygan"
#define DGL_PRODUCT "Debugler"

/**
 * Macros for QSettings variable names
 */
#define STRINGIFY(X) #X
#define SLASHIFY(X, Y) STRINGIFY(X / Y)
#define DGL_SETTINGS(X) SLASHIFY(widgets, X)
#define DGL_GEOMETRY_SETTINGS DGL_SETTINGS(geometry)
#define DGL_WINDOW_STATE_SETTINGS DGL_SETTINGS(windowState)
#define DGL_ColorScheme_SETTINGS DGL_SETTINGS(ColorScheme)
#define DGL_ADB_PATH_SETTINGS DGL_SETTINGS(AdbPath)

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
        : QMainWindow(_parent, flags), m_process(NULL) {

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

    debugeeInfo("");

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
}

DGLMainWindow::~DGLMainWindow() {}

void DGLMainWindow::closeEvent(QCloseEvent *_event) {

    if (m_controller.isConnected() &&
        QMessageBox::question(
            this, "Confirm close",
            "Debugging session is in progress. Close application?",
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        _event->ignore();

    } else {
        disconnect();

        // store QSettings

        QSettings settings(DGL_COMPANY, DGL_PRODUCT);
        settings.setValue(DGL_GEOMETRY_SETTINGS, saveGeometry());
        settings.setValue(DGL_WINDOW_STATE_SETTINGS, saveState());
        settings.setValue(DGL_ColorScheme_SETTINGS, m_ColorScheme);
        settings.setValue(DGL_ADB_PATH_SETTINGS,
                          QString::fromStdString(
                              DGLAdbInterface::get()->getAdbPath()).toUtf8());

        // Send even to parrent class

        _event->accept();
    }
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
}

void DGLMainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(runAct);
    fileMenu->addAction(attachAct);
    fileMenu->addAction(attachAndroidAct);
    fileMenu->addAction(disconnectAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    debugMenu = menuBar()->addMenu(tr("&Debug"));
    debugMenu->addAction(debugContinueAct);
    debugMenu->addAction(debugInterruptAct);
    debugMenu->addAction(debugStepAct);
    debugMenu->addAction(debugStepDrawCallAct);
    debugMenu->addAction(debugStepFrameAct);
    debugMenu->addAction(addDeleteBreakPointsAct);
    debugMenu->addSeparator();
    debugMenu->addAction(setBreakOnGLErrorAct);
    debugMenu->addAction(setBreakOnDebugOutputAct);
    debugMenu->addAction(setBreakOnCompilerErrAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addSeparator();
    ColorSchemeMenu = viewMenu->addMenu(tr("Color Schemes"));
    for (uint i = 0; i < DGLNUM_COLOR_SCHEMES; i++) {
        ColorSchemeMenu->addAction(setColorSchemeActs[i]);
    }

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(configurationAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void DGLMainWindow::createToolBars() {
    debugToolBar = addToolBar(tr("Debug"));
    debugToolBar->addAction(debugContinueAct);
    debugToolBar->addAction(debugInterruptAct);
    debugToolBar->addAction(debugStepAct);
    debugToolBar->addAction(debugStepDrawCallAct);
    debugToolBar->addAction(debugStepFrameAct);
    debugToolBar->addAction(addDeleteBreakPointsAct);
    debugToolBar->addSeparator();
    debugToolBar->addAction(setBreakOnGLErrorAct);
    debugToolBar->addAction(setBreakOnDebugOutputAct);
    debugToolBar->addAction(setBreakOnCompilerErrAct);
}

void DGLMainWindow::createStatusBar() {

    // set initial status

    statusBar()->showMessage(tr("Ready"));
}

void DGLMainWindow::createActions() {

    // create "QActions" - bindings between mainwindow clickable widgets, and
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

    runAct = new QAction(tr("&Run application ..."), this);
    runAct->setStatusTip(tr("Opens run application dialog window"));
    CONNASSERT(runAct, SIGNAL(triggered()), this, SLOT(runDialog()));
    runAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    attachAct = new QAction(tr("&Attach to"), this);
    attachAct->setStatusTip(tr("Attach to IP target"));
    CONNASSERT(attachAct, SIGNAL(triggered()), this, SLOT(attach()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), attachAct,
               SLOT(setDisabled(bool)));
    attachAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));

    attachAndroidAct = new QAction(tr("&Attach to Android App"), this);
    attachAndroidAct->setStatusTip(tr("Attach to IP target"));
    CONNASSERT(attachAndroidAct, SIGNAL(triggered()), this,
               SLOT(attachAndroidApp()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), attachAndroidAct,
               SLOT(setDisabled(bool)));

    disconnectAct = new QAction(tr("&Disconnect"), this);
    disconnectAct->setStatusTip(tr("Disconnect an terminate application"));
    CONNASSERT(disconnectAct, SIGNAL(triggered()), this, SLOT(disconnect()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), disconnectAct,
               SLOT(setEnabled(bool)));
    disconnectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    disconnectAct->setEnabled(false);

    debugContinueAct = new QAction(tr("&Continue"), this);
    debugContinueAct->setStatusTip(tr("Continue program execution"));
    CONNASSERT(debugContinueAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugContinue()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugContinueAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugContinueAct,
               SLOT(setEnabled(bool)));
    debugContinueAct->setShortcut(QKeySequence(Qt::Key_F5));
    debugContinueAct->setEnabled(false);

    debugInterruptAct = new QAction(tr("&Interrupt (on GL)"), this);
    debugInterruptAct->setStatusTip(
        tr("Interrupt program execution on GL call"));
    CONNASSERT(debugInterruptAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugInterrupt()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugInterruptAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setRunning(bool)), debugInterruptAct,
               SLOT(setEnabled(bool)));
    debugInterruptAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F5));
    debugInterruptAct->setEnabled(false);

    debugStepAct = new QAction(tr("&Step into"), this);
    debugStepAct->setStatusTip(tr("Step one GL call"));
    CONNASSERT(debugStepAct, SIGNAL(triggered()), &m_controller,
               SLOT(debugStep()));
    CONNASSERT(&m_controller, SIGNAL(setConnected(bool)), debugStepAct,
               SLOT(setEnabled(bool)));
    CONNASSERT(&m_controller, SIGNAL(setBreaked(bool)), debugStepAct,
               SLOT(setEnabled(bool)));
    debugStepAct->setShortcut(QKeySequence(Qt::Key_F11));
    debugStepAct->setEnabled(false);

    debugStepDrawCallAct = new QAction(tr("&Drawcall step"), this);
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

    setBreakOnGLErrorAct->setCheckable(true);
    setBreakOnGLErrorAct->setChecked(m_controller.getConfig().m_BreakOnGLError);
    CONNASSERT(setBreakOnGLErrorAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    setBreakOnDebugOutputAct = new QAction(tr("Break on debug output"), this);
    setBreakOnDebugOutputAct->setStatusTip(
        tr("Break execution on debug output message"));

    // this action has a state - it is checbox-like checkable

    setBreakOnDebugOutputAct->setCheckable(true);
    setBreakOnDebugOutputAct->setChecked(
        m_controller.getConfig().m_BreakOnGLError);
    CONNASSERT(setBreakOnDebugOutputAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    setBreakOnCompilerErrAct =
        new QAction(tr("Break on compiler/linker error"), this);
    setBreakOnCompilerErrAct->setStatusTip(
        tr("Break execution on debug GLSL compiler or linker error"));

    // this action has a state - it is checbox-like checkable

    setBreakOnCompilerErrAct->setCheckable(true);
    setBreakOnCompilerErrAct->setChecked(
        m_controller.getConfig().m_BreakOnCompilerError);
    CONNASSERT(setBreakOnCompilerErrAct, SIGNAL(toggled(bool)), this,
               SLOT(setBreakOnWhatever(bool)));

    // Only one color scheme can be choosed - put all related actions to action
    // group

    setColorSchemeActGroup = new QActionGroup(this);

    // iterate through all color schemes. for each one create one action

    for (uint i = 0; i < DGLNUM_COLOR_SCHEMES; i++) {
        setColorSchemeActs[i] = new QAction(tr(dglColorSchemes[i].name), this);
        setColorSchemeActs[i]->setCheckable(true);
        setColorSchemeActs[i]->setActionGroup(setColorSchemeActGroup);
        setColorSchemeActs[i]->setStatusTip(tr("Set this color scheme"));

        // connect all color scheme actions to one mapper, so we can connect it
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
}

void DGLMainWindow::createInteractions() {

    // connect some signals from DGLcontroller to UI

    CONNASSERT(&m_controller, SIGNAL(newStatus(const QString &)),
               m_ui.statusBar, SLOT(showMessage(const QString &)));
    CONNASSERT(&m_controller,
               SIGNAL(connectionLost(const QString &, const QString &)), this,
               SLOT(connectionLost(const QString &, const QString &)));
    CONNASSERT(&m_controller, SIGNAL(debugeeInfo(const std::string &)), this,
               SLOT(debugeeInfo(const std::string &)));
}

void DGLMainWindow::readSettings() {

    // read settings

    QSettings settings(DGL_COMPANY, DGL_PRODUCT);
    restoreGeometry(settings.value(DGL_GEOMETRY_SETTINGS).toByteArray());
    restoreState(settings.value(DGL_WINDOW_STATE_SETTINGS).toByteArray());

    DGLAdbInterface::get()->setAdbPath(
        settings.value(DGL_ADB_PATH_SETTINGS).toString().toStdString());

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

void DGLMainWindow::configure() {
    DGLConfigDialog dialog(m_controller.getConfig(),
                           DGLAdbInterface::get()->getAdbPath());
    if (dialog.exec() == QDialog::Accepted) {
        m_controller.sendConfig(dialog.getConfig());
        DGLAdbInterface::get()->setAdbPath(dialog.getAdbPath().toStdString());
    }
}

void DGLMainWindow::debugeeInfo(const std::string &processName) {
    if (processName.length()) {
        setWindowTitle(QString::fromStdString("Debugler - " + processName));
    } else {
        setWindowTitle("Debugler - disconnected");
    }
}

void DGLMainWindow::about() {
    QMessageBox::about(
        this, tr("About Debuggler"),
        tr("<b>Debugler " DGL_VERSION
           "</b>, The OpenGL debugger<br/><br/>"
           "Copyright (C) 2013 Slawomir Cygan.<br/><br/> "
           "<a href=\"https://github.com/debugler/debugler\"/>"));
}

void DGLMainWindow::attach() {

    // execute connection dialog to obtain connection parameters

    if (m_ConnectDialog.exec() == QDialog::Accepted) {

        // if dialog is successfull, initialte connection in DGLController

        m_controller.connectServer(m_ConnectDialog.getAddress(),
                                   m_ConnectDialog.getPort());
    }
}

void DGLMainWindow::attachAndroidApp() {

    // execute connection dialog to obtain connection parameters

    if (m_ConnectAndroidDialog.exec() == QDialog::Accepted) {
        throw std::runtime_error("unimplemented");
    }
}

void DGLMainWindow::runDialog() {

    // execute connection dialog to obtain connection parameters

    if (m_RunDialog.exec() == QDialog::Accepted) {

        disconnect();

        try {

            m_BusyDialog = std::make_shared<QProgressDialog>(
                "Starting debugging session (waiting for application to try "
                "use OpenGL)...",
                "Cancel", 0, 1, this);

            m_BusyDialog->setWindowModality(Qt::WindowModal);
            m_BusyDialog->setValue(0);

            QProgressBar *bar = new QProgressBar(m_BusyDialog.get());
            m_BusyDialog->setBar(bar);
            bar->setMaximum(0);
            bar->setMinimum(0);
            bar->setValue(-1);
            CONNASSERT(m_BusyDialog.get(), SIGNAL(canceled()), this,
                       SLOT(disconnect()));

            // randomize connection port
            int port = rand() % (0xffff - 1024) + 1024;

            m_process = new DGLDebugeeQTProcess(port, m_RunDialog.getModeEGL());

            m_process->setParent(this);

            CONNASSERT(m_process, SIGNAL(processReady()), this,
                       SLOT(processReadyHandler()));
            CONNASSERT(m_process, SIGNAL(processError(std::string)), this,
                       SLOT(processErrorHandler(std::string)));
            CONNASSERT(m_process, SIGNAL(processFinished(int)), this,
                       SLOT(processExitHandler(int)));
            CONNASSERT(m_process, SIGNAL(processCrashed()), this,
                       SLOT(processCrashHandler()));

            m_process->run(m_RunDialog.getExecutable(), m_RunDialog.getPath(),
                           m_RunDialog.getCommandLineArgs());
        }
        catch (const std::runtime_error &err) {
            if (m_process) {
                m_process->requestDelete();
                m_process = NULL;
            }
            QMessageBox::critical(NULL, tr("Fatal Error"),
                                  QString::fromStdString(err.what()));
        }
    }
}

void DGLMainWindow::processCrashHandler() {
    disconnect();
    QMessageBox::critical(this, tr("Process crashed"),
                          tr("Loader process has crashed."));
}

void DGLMainWindow::processErrorHandler(std::string err) {
    disconnect();
    QMessageBox::critical(this, tr("Fatal Error"), QString::fromStdString(err));
}

void DGLMainWindow::processExitHandler(int code) {
    disconnect();
    QMessageBox::information(
        NULL, tr("Process Exited"),
        tr("Process has exited with code ") + QString::number(code) + ".");
}

void DGLMainWindow::processReadyHandler() {

    m_BusyDialog->reset();

    std::ostringstream portStr;
    portStr << m_process->getPort();
    m_controller.connectServer("127.0.0.1", portStr.str());
}

void DGLMainWindow::disconnect() {
    m_controller.disconnectServer();
    if (m_process) {
        m_process->requestDelete();
        m_process = NULL;
    }
    m_BusyDialog.reset();
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

    m_controller.getConfig().m_BreakOnGLError =
        setBreakOnGLErrorAct->isChecked();
    m_controller.getConfig().m_BreakOnDebugOutput =
        setBreakOnDebugOutputAct->isChecked();
    m_controller.getConfig().m_BreakOnCompilerError =
        setBreakOnCompilerErrAct->isChecked();
    m_controller.sendConfig();
    showConfig();
}

void DGLMainWindow::connectionLost(const QString &title, const QString &msg) {
    if (m_process && m_controller.isConnected()) {
        // we still have a process, that should terminate now

        // m_process has still handler slots connected here, so user
        // will be informed about process exit.
        m_process->exit(false);
    } else {
        disconnect();
        QMessageBox::critical(this, title, msg);
    }
}

void DGLMainWindow::showConfig() {
    setBreakOnGLErrorAct->setChecked(m_controller.getConfig().m_BreakOnGLError);
    setBreakOnDebugOutputAct->setChecked(
        m_controller.getConfig().m_BreakOnDebugOutput);
    setBreakOnCompilerErrAct->setChecked(
        m_controller.getConfig().m_BreakOnCompilerError);
}
