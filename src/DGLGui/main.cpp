#include "dglmainwindow.h"
#include <QtGui/QApplication>
#include <QMessageBox>
#include <QFile>

bool initializeTemporaries() {

    const char* files [] = {
        "c_comment.lang", 
        "c_string.lang", 
        "default.lang", 
        "default.style", 
        "function.lang", 
        "glsl.lang", 
        "html_simple.lang", 
        "key_string.lang", 
        "number.lang", 
        "symbols.lang", 
        "todo.lang", 
        "url.lang",
        "xml.lang"
    };

    try {
        for (size_t i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
            QFile in((std::string(":highlight/") + files[i]).c_str());
            QFile out(files[i]);
            if (!in.open(QIODevice::ReadOnly) || !out.open(QIODevice::WriteOnly)) {
                throw 1;
            }
            out.write(in.readAll());
        }
    } catch(...) {
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!initializeTemporaries()) {
        QMessageBox::critical(NULL, "Fatal Error",
            "Extraction of temporary files failed.");
        return EXIT_FAILURE;
    }

    DGLMainWindow w;
    w.show();
    return qApp->exec();
}

