#include <QMessageBox>
#include <QSettings>

#include "dglmainwindow.h"
#include "dglconnectdialog.h"
#include "dglbreakpointdialog.h"
#include "dgltraceview.h"
#include "dgltreeview.h"
#include "dgltextureview.h"
#include "dglbufferview.h"
#include "dglframebufferview.h"
#include "dglbreakpointview.h"
#include "dglgui.h"

#define DGL_COMPANY "sacygan"
#define DGL_PRODUCT "debuggler"

DGLMainWindow::DGLMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags) {
    m_ui.setupUi(this);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();
    createInteractions();
    readSettings();
}

DGLMainWindow::~DGLMainWindow() {

}

void DGLMainWindow::closeEvent(QCloseEvent *event) {
    QSettings settings(DGL_COMPANY, DGL_PRODUCT);
    settings.setValue("widgets/geometry", saveGeometry());
    settings.setValue("widgets/windowState", saveState());
    QMainWindow::closeEvent(event);
}


void DGLMainWindow::createDockWindows() {
    {
        QDockWidget *dock = new DGLTraceView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLTreeView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLTextureView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLBufferView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLBreakPointView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    } {
        QDockWidget *dock = new DGLFramebufferView(this, &m_controller);
        addDockWidget(Qt::AllDockWidgetAreas, dock);
        viewMenu->addAction(dock->toggleViewAction());
    }

     /*connect(customerList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(insertCustomer(QString)));
     connect(paragraphsList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(addParagraph(QString)));*/
}

void DGLMainWindow::createMenus() {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(attachAct);
     fileMenu->addAction(disconnectAct);
     fileMenu->addSeparator();
     fileMenu->addAction(quitAct);

     //editMenu = menuBar()->addMenu(tr("&Edit"));
     //editMenu->addAction(undoAct);*/

     debugMenu = menuBar()->addMenu(tr("&Debug"));
     debugMenu->addAction(debugContinueAct);
     debugMenu->addAction(debugInterruptAct);
     debugMenu->addAction(debugStepAct);
     debugMenu->addAction(addDeleteBreakPointsAct);


     viewMenu = menuBar()->addMenu(tr("&View"));

     menuBar()->addSeparator();

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
     
 }


void DGLMainWindow::createToolBars() {
     /*fileToolBar = addToolBar(tr("File"));
     fileToolBar->addAction(newLetterAct);
     fileToolBar->addAction(saveAct);
     fileToolBar->addAction(printAct);

     editToolBar = addToolBar(tr("Edit"));
     editToolBar->addAction(undoAct);*/
 }

 void DGLMainWindow::createStatusBar() {
     statusBar()->showMessage(tr("Ready"));
 }

 void DGLMainWindow::createActions() {
     quitAct = new QAction(tr("&Quit"), this);
     quitAct->setShortcuts(QKeySequence::Quit);
     quitAct->setStatusTip(tr("Quit the application"));
     CONNASSERT(connect(quitAct, SIGNAL(triggered()), this, SLOT(close())));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     CONNASSERT(connect(aboutAct, SIGNAL(triggered()), this, SLOT(about())));

     attachAct = new QAction(tr("&Attach to"), this);
     attachAct->setStatusTip(tr("Attach to IP target"));
     CONNASSERT(connect(attachAct, SIGNAL(triggered()), this, SLOT(attach())));

     disconnectAct = new QAction(tr("&Disconnect"), this);
     disconnectAct->setStatusTip(tr("Disconnect an terminate application"));
     CONNASSERT(connect(disconnectAct, SIGNAL(triggered()), this, SLOT(disconnect())));

     debugContinueAct = new QAction(tr("&Continue"), this);
     debugContinueAct->setStatusTip(tr("Continue program execution"));
     CONNASSERT(connect(debugContinueAct, SIGNAL(triggered()), &m_controller, SLOT(debugContinue())));

     debugInterruptAct = new QAction(tr("&Interrupt (on GL)"), this);
     debugInterruptAct->setStatusTip(tr("Interrupt program execution on GL call"));
     CONNASSERT(connect(debugInterruptAct, SIGNAL(triggered()), &m_controller, SLOT(debugInterrupt())));

     debugStepAct = new QAction(tr("&Step call"), this);
     debugStepAct->setStatusTip(tr("Step one GL call"));
     CONNASSERT(connect(debugStepAct, SIGNAL(triggered()), &m_controller, SLOT(debugStep())));

     addDeleteBreakPointsAct = new QAction(tr("&Breakpoints..."), this);
     addDeleteBreakPointsAct->setStatusTip(tr("Add or remove breakpoints"));
     CONNASSERT(connect(addDeleteBreakPointsAct, SIGNAL(triggered()), this, SLOT(addDeleteBreakPoints())));

 }

  void DGLMainWindow::createInteractions() {
      CONNASSERT(connect(&m_controller, SIGNAL(newStatus(const QString&)), m_ui.statusBar, SLOT(showMessage(const QString&))));
      CONNASSERT(connect(&m_controller, SIGNAL(error(const QString&, const QString&)), this, SLOT(errorMessage(const QString&, const QString&))));
  }

  void DGLMainWindow::readSettings() {
      QSettings settings(DGL_COMPANY, DGL_PRODUCT);
      restoreGeometry(settings.value("widgets/geometry").toByteArray());
      restoreState(settings.value("widgets/windowState").toByteArray());
  }

  void DGLMainWindow::about() {
    QMessageBox::about(this, tr("About Debuggler"),
             tr("The <b>Debuggler</b>, OpenGL debugger<br><br> Slawomir Cygan: Eng. thesis,"
                "Gdansk University of Technology<br>"
                "Faculty of Electronics, Telecommunications and Informatics<br>"
                "Department of Computer Architecture, 2012."));
 }

 void DGLMainWindow::attach() {
     DGLConnectDialog dialog;
     if (dialog.exec() == QDialog::Accepted) {
         m_controller.connectServer(dialog.getAddress(), dialog.getPort());
     }
 }

 void DGLMainWindow::disconnect() {
     m_controller.disconnectServer();
 }

 void DGLMainWindow::addDeleteBreakPoints() {
      DGLBreakPointDialog dialog(&m_controller);
      if (dialog.exec() == QDialog::Accepted) {
          m_controller.getBreakPoints()->setCurrent(dialog.getBreakPoints());
      }
 }

void DGLMainWindow::errorMessage(const QString& title, const QString& msg) {
    QMessageBox::critical(this, title, msg);
}