#include <QMessageBox>

#include "dglmainwindow.h"
#include "dglconnectdialog.h"
#include "dgltraceview.h"

DGLMainWindow::DGLMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags) {
    m_ui.setupUi(this);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();
    createInteractions();
}

DGLMainWindow::~DGLMainWindow() {

}


void DGLMainWindow::createDockWindows() {
     QDockWidget *dock = new DGLTraceView(this, &m_controller);
     addDockWidget(Qt::RightDockWidgetArea, dock);
     viewMenu->addAction(dock->toggleViewAction());

     /*connect(customerList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(insertCustomer(QString)));
     connect(paragraphsList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(addParagraph(QString)));*/
 }

void DGLMainWindow::createMenus() {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(attachAct);
     fileMenu->addSeparator();
     fileMenu->addAction(quitAct);

     //editMenu = menuBar()->addMenu(tr("&Edit"));
     //editMenu->addAction(undoAct);*/

     debugMenu = menuBar()->addMenu(tr("&Debug"));
     debugMenu->addAction(debugContinueAct);
     debugMenu->addAction(debugInterruptAct);
     debugMenu->addAction(debugStepAct);


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
     assert(connect(quitAct, SIGNAL(triggered()), this, SLOT(close())));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     assert(connect(aboutAct, SIGNAL(triggered()), this, SLOT(about())));

     attachAct = new QAction(tr("&Attach to"), this);
     attachAct->setStatusTip(tr("Attach to IP target"));
     assert(connect(attachAct, SIGNAL(triggered()), this, SLOT(attach())));

     debugContinueAct = new QAction(tr("&Continue"), this);
     debugContinueAct->setStatusTip(tr("Continue program execution"));
     assert(connect(debugContinueAct, SIGNAL(triggered()), &m_controller, SLOT(debugContinue())));

     debugInterruptAct = new QAction(tr("&Interrupt (on GL)"), this);
     debugInterruptAct->setStatusTip(tr("Interrupt program execution on GL call"));
     assert(connect(debugInterruptAct, SIGNAL(triggered()), &m_controller, SLOT(debugInterrupt())));

     debugStepAct = new QAction(tr("&Step call"), this);
     debugStepAct->setStatusTip(tr("Step one GL call"));
     assert(connect(debugStepAct, SIGNAL(triggered()), &m_controller, SLOT(debugStep())));

 }

  void DGLMainWindow::createInteractions() {
      assert(connect(&m_controller, SIGNAL(newStatus(const QString&)), m_ui.statusBar, SLOT(showMessage(const QString&))));
      assert(connect(&m_controller, SIGNAL(error(const QString&, const QString&)), this, SLOT(errorMessage(const QString&, const QString&))));
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
          m_controller.connectClient(dialog.getAddress(), dialog.getPort());
      }
  }


void DGLMainWindow::errorMessage(const QString& title, const QString& msg) {
    QMessageBox::critical(this, title, msg);
}