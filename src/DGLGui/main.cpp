#include "dglmainwindow.h"
#include <QtGui/QApplication>

#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

int main(int argc, char *argv[])
{

#ifdef _WIN32             
     srand(GetTickCount());
#else
     timeval t1;
     gettimeofday(&t1, NULL);
     srand(t1.tv_usec * t1.tv_sec);
#endif             


    QApplication app(argc, argv);   

    DGLMainWindow w;
    w.show();
    return qApp->exec();
}

