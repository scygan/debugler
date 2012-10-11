#include <QDockWidget>
#include <QMessageBox>

#include "dglmainwindow.h"


DGLMainWindow::DGLMainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags) {
    ui.setupUi(this);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWindows();
}

DGLMainWindow::~DGLMainWindow() {

}


void DGLMainWindow::createDockWindows() {
     QDockWidget *dock = new QDockWidget(tr("Call trace"), this);
     dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
     customerList = new QListWidget(dock);
     customerList->addItems(QStringList()
             << "wutuf");
     dock->setWidget(customerList);
     addDockWidget(Qt::RightDockWidgetArea, dock);
     viewMenu->addAction(dock->toggleViewAction());

     /*connect(customerList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(insertCustomer(QString)));
     connect(paragraphsList, SIGNAL(currentTextChanged(QString)),
             this, SLOT(addParagraph(QString)));*/
 }

void DGLMainWindow::createMenus() {
     fileMenu = menuBar()->addMenu(tr("&File"));
     //fileMenu->addAction(newLetterAct);
     //fileMenu->addAction(saveAct);
     //fileMenu->addAction(printAct);
     fileMenu->addSeparator();
     fileMenu->addAction(quitAct);

     //editMenu = menuBar()->addMenu(tr("&Edit"));
     //editMenu->addAction(undoAct);*/

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
     connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

     aboutAct = new QAction(tr("&About"), this);
     aboutAct->setStatusTip(tr("Show the application's About box"));
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
 }

  void DGLMainWindow::about() {
    QMessageBox::about(this, tr("About Debuggler"),
             tr("The <b>Debuggler</b>, OpenGL debugger<br><br> Slawomir Cygan: Eng. thesis,"
                "Gdansk University of Technology<br>"
                "Faculty of Electronics, Telecommunications and Informatics<br>"
                "Department of Computer Architecture, 2012."));
 }
