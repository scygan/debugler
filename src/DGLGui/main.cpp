#include "dglmainwindow.h"
#include <QtGui/QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);   

    DGLMainWindow w;
    w.show();
    return qApp->exec();
}

