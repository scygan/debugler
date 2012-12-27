#include <QMessageBox>
#include <QSettings>
#include <QFile>

#include "dglmainwindow.h"
#include "dglbreakpointdialog.h"
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
#include "dglgui.h"

#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "CompleteInject.h"

/**
 * Macros indentifying entity, that stores & loads QSettings
 */
#define DGL_COMPANY "SaCygan"
#define DGL_PRODUCT "Debugler"

/** 
 * Macros for QSettings variable names
 */
#define STRINGIFY(X) #X
#define DGL_SETTINGS(X) STRINGIFY(widgets/##X)
#define DGL_GEOMETRY_SETTINGS DGL_SETTINGS(geometry)
#define DGL_WINDOW_STATE_SETTINGS DGL_SETTINGS(windowState)
#define DGL_ColorScheme_SETTINGS DGL_SETTINGS(ColorScheme)


/**
 * Array of available main window color settings
 */
struct DGLColorScheme {
    char* name; /**< Display name */
    char* file; /**< Resource file name wyth styleshet */
} dglColorSchemes[DGLNUM_COLOR_SCHEMES] = {
    { "Default",     ":/res/default.stylesheet"   },
    { "Dark Orange", ":/res/darkorange.stylesheet"},
};

DGLMainWindow::DGLMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags) {

    //load designer UI 
    
    m_ui.setupUi(this);
    setCentralWidget(NULL);
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
}

DGLMainWindow::~DGLMainWindow() {

}

void DGLMainWindow::closeEvent(QCloseEvent *event) {

    if (m_controller.isConnected() && QMessageBox::question(this, "Confirm close", "Debugging session is in progress. Close application?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        event->ignore();

    } else {
        m_controller.disconnectServer();

        //store QSettings

        QSettings settings(DGL_COMPANY, DGL_PRODUCT);
        settings.setValue(DGL_GEOMETRY_SETTINGS, saveGeometry());
        settings.setValue(DGL_WINDOW_STATE_SETTINGS, saveState());
        settings.setValue(DGL_ColorScheme_SETTINGS, m_ColorScheme);

        //Send even to parrent class

        event->accept();
    }
}


void DGLMainWindow::createDockWindows() {
    
    //Create all dock windows.
    
    {
        QDockWidget *dock = new DGLTraceView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLTreeView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLTextureView(this, &m_controller);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setMinimumSize(QSize(600, 0));
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLBufferView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLFramebufferView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLFBOView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLShaderView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLProgramView(this, &m_controller);
        dock->setMinimumSize(QSize(600, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLGPUView(this, &m_controller);
        dock->setMinimumSize(QSize(0, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLStateView(this, &m_controller);
        dock->setMinimumSize(QSize(0, 0));
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }

}

void DGLMainWindow::createMenus() {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(runAct);
     fileMenu->addAction(attachAct);
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
     
     //set initial status
     
     statusBar()->showMessage(tr("Ready"));
 }

 void DGLMainWindow::createActions() {

     //create "QActions" - bindings between mainwindow clickable widgets, and local slots

     quitAct = new QAction(tr("&Quit"), this);
     quitAct->setShortcuts(QKeySequence::Quit);
     quitAct->setStatusTip(tr("Quit the application"));
     CONNASSERT(connect(quitAct, SIGNAL(triggered()), this, SLOT(close())));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     CONNASSERT(connect(aboutAct, SIGNAL(triggered()), this, SLOT(about())));

     runAct = new QAction(tr("&Run application ..."), this);
     runAct->setStatusTip(tr("Opens run application dialog window"));
     CONNASSERT(connect(runAct, SIGNAL(triggered()), this, SLOT(runDialog())));

     attachAct = new QAction(tr("&Attach to"), this);
     attachAct->setStatusTip(tr("Attach to IP target"));
     CONNASSERT(connect(attachAct, SIGNAL(triggered()), this, SLOT(attach())));
     CONNASSERT(connect(&m_controller, SIGNAL(setDisconnected(bool)), attachAct, SLOT(setEnabled(bool))));

     disconnectAct = new QAction(tr("&Disconnect"), this);
     disconnectAct->setStatusTip(tr("Disconnect an terminate application"));
     CONNASSERT(connect(disconnectAct, SIGNAL(triggered()), this, SLOT(disconnect())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), disconnectAct, SLOT(setEnabled(bool))));
     disconnectAct->setEnabled(false);

     debugContinueAct = new QAction(tr("&Continue"), this);
     debugContinueAct->setStatusTip(tr("Continue program execution"));
     CONNASSERT(connect(debugContinueAct, SIGNAL(triggered()), &m_controller, SLOT(debugContinue())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), debugContinueAct, SLOT(setEnabled(bool))));
     CONNASSERT(connect(&m_controller, SIGNAL(setBreaked(bool)), debugContinueAct, SLOT(setEnabled(bool))));
     debugContinueAct->setEnabled(false);

     debugInterruptAct = new QAction(tr("&Interrupt (on GL)"), this);
     debugInterruptAct->setStatusTip(tr("Interrupt program execution on GL call"));
     CONNASSERT(connect(debugInterruptAct, SIGNAL(triggered()), &m_controller, SLOT(debugInterrupt())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), debugInterruptAct, SLOT(setEnabled(bool))));
     CONNASSERT(connect(&m_controller, SIGNAL(setRunning(bool)), debugInterruptAct, SLOT(setEnabled(bool))));
     debugInterruptAct->setEnabled(false);

     debugStepAct = new QAction(tr("&Step into"), this);
     debugStepAct->setStatusTip(tr("Step one GL call"));
     CONNASSERT(connect(debugStepAct, SIGNAL(triggered()), &m_controller, SLOT(debugStep())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), debugStepAct, SLOT(setEnabled(bool))));
     CONNASSERT(connect(&m_controller, SIGNAL(setBreaked(bool)), debugStepAct, SLOT(setEnabled(bool))));
     debugStepAct->setEnabled(false);

     debugStepDrawCallAct = new QAction(tr("&Drawcall step"), this);
     debugStepDrawCallAct->setStatusTip(tr("Step one GL drawing call"));
     CONNASSERT(connect(debugStepDrawCallAct, SIGNAL(triggered()), &m_controller, SLOT(debugStepDrawCall())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), debugStepDrawCallAct, SLOT(setEnabled(bool))));
     CONNASSERT(connect(&m_controller, SIGNAL(setBreaked(bool)), debugStepDrawCallAct, SLOT(setEnabled(bool))));
     debugStepDrawCallAct->setEnabled(false);

     debugStepFrameAct = new QAction(tr("&Frame step"), this);
     debugStepFrameAct->setStatusTip(tr("Step one GL frame"));
     CONNASSERT(connect(debugStepFrameAct, SIGNAL(triggered()), &m_controller, SLOT(debugStepFrame())));
     CONNASSERT(connect(&m_controller, SIGNAL(setConnected(bool)), debugStepFrameAct, SLOT(setEnabled(bool))));
     CONNASSERT(connect(&m_controller, SIGNAL(setBreaked(bool)), debugStepFrameAct, SLOT(setEnabled(bool))));
     debugStepFrameAct->setEnabled(false);

     addDeleteBreakPointsAct = new QAction(tr("&Breakpoints..."), this);
     addDeleteBreakPointsAct->setStatusTip(tr("Add or remove breakpoints"));
     CONNASSERT(connect(addDeleteBreakPointsAct, SIGNAL(triggered()), this, SLOT(addDeleteBreakPoints())));

     setBreakOnGLErrorAct = new QAction(tr("Break on GL error"), this);
     setBreakOnGLErrorAct->setStatusTip(tr("Break execution on GL error (glGetError() != GL_NO_ERROR)"));

     //this action has a state - it is checbox-like checkable

     setBreakOnGLErrorAct->setCheckable(true);
     setBreakOnGLErrorAct->setChecked(m_controller.getConfig().m_BreakOnGLError);
     CONNASSERT(connect(setBreakOnGLErrorAct, SIGNAL(toggled(bool)), this, SLOT(setBreakOnWhatever(bool))));

     setBreakOnDebugOutputAct = new QAction(tr("Break on debug output"), this);
     setBreakOnDebugOutputAct->setStatusTip(tr("Break execution on debug output message"));

     //this action has a state - it is checbox-like checkable

     setBreakOnDebugOutputAct->setCheckable(true);
     setBreakOnDebugOutputAct->setChecked(m_controller.getConfig().m_BreakOnGLError);
     CONNASSERT(connect(setBreakOnDebugOutputAct, SIGNAL(toggled(bool)), this, SLOT(setBreakOnWhatever(bool))));

     setBreakOnCompilerErrAct = new QAction(tr("Break on compiler/linker error"), this);
     setBreakOnCompilerErrAct->setStatusTip(tr("Break execution on debug GLSL compiler or linker error"));

     //this action has a state - it is checbox-like checkable

     setBreakOnCompilerErrAct->setCheckable(true);
     setBreakOnCompilerErrAct->setChecked(m_controller.getConfig().m_BreakOnCompilerError);
     CONNASSERT(connect(setBreakOnCompilerErrAct, SIGNAL(toggled(bool)), this, SLOT(setBreakOnWhatever(bool))));
     
    
     //Only one color scheme can be choosed - put all related actions to action group

     setColorSchemeActGroup = new QActionGroup(this);

     //iterate through all color schemes. for each one create one action

     for (uint i = 0; i < DGLNUM_COLOR_SCHEMES; i++) {
         setColorSchemeActs[i] = new QAction(tr(dglColorSchemes[i].name), this);
         setColorSchemeActs[i]->setCheckable(true);
         setColorSchemeActs[i]->setActionGroup(setColorSchemeActGroup);
         setColorSchemeActs[i]->setStatusTip(tr("Set this color scheme"));

         //connect all color scheme actions to one mapper, so we can connect it later to only one signal

         m_SetColorSchemeSignalMapper.setMapping(setColorSchemeActs[i], i);
         CONNASSERT(connect(setColorSchemeActs[i], SIGNAL(triggered()), &m_SetColorSchemeSignalMapper, SLOT(map())));
     }

     //mapper maps connected actions to one emitted signal by int parameter. Connect this signal to "this"

     CONNASSERT(connect(&m_SetColorSchemeSignalMapper, SIGNAL(mapped(int)), this, SLOT(setColorScheme(int))));
 }

  void DGLMainWindow::createInteractions() {

      //connect some signals from DGLcontroller to UI

      CONNASSERT(connect(&m_controller, SIGNAL(newStatus(const QString&)), m_ui.statusBar, SLOT(showMessage(const QString&))));
      CONNASSERT(connect(&m_controller, SIGNAL(error(const QString&, const QString&)), this, SLOT(errorMessage(const QString&, const QString&))));
      CONNASSERT(connect(&m_controller, SIGNAL(debugeeInfo(const std::string&)), this, SLOT(debugeeInfo(const std::string&))));
  }

  void DGLMainWindow::readSettings() {

      //read settings

      QSettings settings(DGL_COMPANY, DGL_PRODUCT);
      restoreGeometry(settings.value(DGL_GEOMETRY_SETTINGS).toByteArray());
      restoreState(settings.value(DGL_WINDOW_STATE_SETTINGS).toByteArray());
      
      //decode and set actual color scheme from settings

      uint ColorScheme = settings.value(DGL_ColorScheme_SETTINGS).toUInt();
      setColorScheme(ColorScheme);
  }

  void DGLMainWindow::setColorScheme(int colorScheme) {

      //check if scheme id is valid

      if (colorScheme >= 0 && colorScheme < DGLNUM_COLOR_SCHEMES) {
          
          //check if apropriate action is already checked, if not check it

          if (!setColorSchemeActs[colorScheme]->isChecked())
              setColorSchemeActs[colorScheme]->setChecked(true);
          
          //store color scheme id

          m_ColorScheme = colorScheme;

          //load color scheme from resource file

          QString fileName(dglColorSchemes[colorScheme].file);
          QFile file(fileName);
          if(file.open(QFile::ReadOnly)) {
              
              //if file exists, read it and set QApplication style

              QString colorSchemeSheet = QLatin1String(file.readAll());
              qApp->setStyleSheet(colorSchemeSheet);
          }
      }
  }

  void DGLMainWindow::debugeeInfo(const std::string& processName) {
      if (processName.length()) {
          setWindowTitle(QString::fromStdString("Debugler - " + processName));
      } else {
          setWindowTitle("Debugler - disconnected");
      }    
  }

  void DGLMainWindow::about() {
    QMessageBox::about(this, tr("About Debuggler"),
             tr("The <b>Debuggler</b>, OpenGL debugger<br><br> Slawomir Cygan: Eng. thesis,"
                "Gdansk University of Technology<br>"
                "Faculty of Electronics, Telecommunications and Informatics<br>"
                "Department of Computer Architecture, 2012."));
 }

 void DGLMainWindow::attach() {

     //execute connection dialog to obtain connection parameters


     if (m_ConnectDialog.exec() == QDialog::Accepted) {

         //if dialog is successfull, initialte connection in DGLController

         m_controller.connectServer(m_ConnectDialog.getAddress(), m_ConnectDialog.getPort());
     }
 }

 void DGLMainWindow::runDialog() {

     //execute connection dialog to obtain connection parameters

     if (m_RunDialog.exec() == QDialog::Accepted) {

         try {
             
             //randomize connection port
             srand(GetTickCount());
             int port = rand() % (0xffff - 1024) + 1024;
             std::stringstream portStr;  portStr << port;

             //set environment variables - child process will inherit these
             SetEnvironmentVariableA("dgl_port", portStr.str().c_str());
             std::string semName = "sem_" + portStr.str();
             SetEnvironmentVariableA("dgl_semaphore", semName.c_str());

             //get wrapper path
             QByteArray baPath = QDir::toNativeSeparators(QFileInfo("DGLWrapper/OpenGL32.dll").absoluteFilePath()).toUtf8();
             const char* wrapperPath = baPath.constData();

             //prepare some structures for CreateProcess output
             STARTUPINFOA startupInfo;
             memset(&startupInfo, 0, sizeof(startupInfo));
             startupInfo.cb = sizeof(startupInfo);
             PROCESS_INFORMATION processInformation; 
             memset(&processInformation, 0, sizeof(processInformation));

             //try run process (suspended - will not run user thread)

             if (CreateProcessA(
                 (LPSTR)m_RunDialog.getExecutable().c_str(),
                 (LPSTR)m_RunDialog.getCommandLineArgs().c_str(),
                 NULL, 
                 NULL,
                 FALSE, 
                 CREATE_SUSPENDED,
                 NULL,
                 m_RunDialog.getPath().c_str(),
                 &startupInfo, 
                 &processInformation) == 0 ) {

                     throw std::runtime_error("Cannot create process");
             }
             
             //process is running, but is suspended (user thread not started, some DLLMain-s may be run by now)

             //inject thread with wrapper library loading code, run through DLLMain and InitializeThread() function

             HANDLE thread = Inject(processInformation.hProcess, wrapperPath, "InitializeThread");

             //wait for remote thread to end - wait for library to load and InitializeThread() to return

             WaitForSingleObject(thread, INFINITE); 

             {
                 //create named IPC semaphore. Application will post it, when server is created (and when OpenGL is first used)

                 boost::interprocess::named_semaphore sem(boost::interprocess::create_only, semName.c_str(), 0);

                 //resume process - now user thread is running
                 //whole OpenGL should be wrapped by now

                 if (ResumeThread(processInformation.hThread) == -1) {
                     throw std::runtime_error("Cannot resume process");
                 }

                 //Wait for application to startup connection server
                 //this may take some time, as we cannot do it on start (cannot setup socket from DLLMain())
                 //we basically wait here for application to call WGL

                 bool timeout; 
                 //boost::posix_time::ptime p = boost::get_system_time() + boost::posix_time::milliseconds(5000);
                 while ((timeout = !sem.timed_wait(boost::get_system_time() + boost::posix_time::milliseconds(5000)))) {

                     //5 seconds is enough. it ususally means that this application is not using OpenGL at all..
                     if (QMessageBox::question(this, tr("Timeout"), tr("The debugger has waited 5 seconds for application to use OpenGL. Wait another 5s?"),
                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
                         break;
                     }
                 }
                 if (timeout) {
                     throw std::runtime_error("Timed out waiting for application to use OpenGL");
                 }
             }

             //application should be running server by now, connect to it and begin debugging

             std::string host = "127.0.0.1";

             m_controller.connectServer(host, portStr.str());

         } catch (const std::runtime_error& err) {

             //generic, GetLastError() aware error printer
 
             if ( DWORD lastError = GetLastError()) {

                 char* errorText;
                 FormatMessageA(
                     FORMAT_MESSAGE_FROM_SYSTEM
                     |FORMAT_MESSAGE_ALLOCATE_BUFFER
                     |FORMAT_MESSAGE_IGNORE_INSERTS,  
                     NULL,
                     GetLastError(),
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPSTR)&errorText,
                     0,
                     NULL);

                 QMessageBox::critical(NULL, "Fatal Error",
                     QString::fromStdString(err.what()) + ": " +  errorText);

                 LocalFree(errorText);

             } else {
                 QMessageBox::critical(NULL, "Fatal Error",
                     QString::fromStdString(err.what()));
             }
         }
     }
 }

 void DGLMainWindow::disconnect() {

     // call DGLcontroller to terminate it's connection

     m_controller.disconnectServer();
 }

 void DGLMainWindow::addDeleteBreakPoints() {

     //execute break point dialog. Already set breakpoint are
     //sourced from provided DGLController object

     DGLBreakPointDialog dialog(&m_controller);
     if (dialog.exec() == QDialog::Accepted) {

         //if dialog was successfull retrieve list bkpoints and feed them
         //to breakpoint controller in DGLcontroller

         m_controller.getBreakPoints()->setCurrent(dialog.getBreakPoints());
     }
 }

 void DGLMainWindow::setBreakOnWhatever(bool) {
     
     //This action enables breaking on various events, like GL error or debug output
     //tell DGLController to configure it's debugee

     DGLConfiguration config(setBreakOnGLErrorAct->isChecked(), setBreakOnDebugOutputAct->isChecked(), setBreakOnCompilerErrAct->isChecked());

     m_controller.configure(config);
 }

void DGLMainWindow::errorMessage(const QString& title, const QString& msg) {

    //all fatals should eventually end here

    QMessageBox::critical(this, title, msg);
}