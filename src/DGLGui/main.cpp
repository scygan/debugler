#include "dglmainwindow.h"
#include <QtGui/QApplication>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QDir>


std::map<std::string, std::string> g_TemporaryFileNameMap;

const char* MangleFileName(const char * name) {
    return g_TemporaryFileNameMap[name].c_str();
}


class BootStrapper {
public:
    BootStrapper(int &argc, char **argv):m_Application(argc, argv) {
        if (!initializeTemporaries()) {
            QMessageBox::critical(NULL, "Fatal Error",
                "Extraction of temporary files failed.");
            exit(EXIT_FAILURE);
        }
    }
    ~BootStrapper() {
        while (m_TemporaryFiles.size()) {
            delete m_TemporaryFiles[m_TemporaryFiles.size() - 1];
            m_TemporaryFiles.pop_back();
        }
    }
private:
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
                m_TemporaryFiles.push_back( new QTemporaryFile(QDir::tempPath() +  files[i]));
                if (!in.open(QIODevice::ReadOnly) || ! m_TemporaryFiles[m_TemporaryFiles.size() - 1]->open()) {
                    throw 1;
                }
                m_TemporaryFiles[m_TemporaryFiles.size() - 1]->write(in.readAll());
                g_TemporaryFileNameMap[std::string("./") + files[i]] =  m_TemporaryFiles[m_TemporaryFiles.size() - 1]->fileName().toStdString();
                m_TemporaryFiles[m_TemporaryFiles.size() - 1]->close();
            }
        } catch(...) {
            return false;
        }
        return true;
    }

    QApplication m_Application;
    std::vector<QTemporaryFile*> m_TemporaryFiles;
};


int main(int argc, char *argv[])
{
    BootStrapper bootstrapper(argc, argv);   

    DGLMainWindow w;
    w.show();
    return qApp->exec();
}

