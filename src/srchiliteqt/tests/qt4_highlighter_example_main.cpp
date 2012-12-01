/*
 * qt4_highlighter_example_main.cpp
 *
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2009
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/Qt4SyntaxHighlighter.h>
#include <srchilite/versions.h>
#include <srchilite/settings.h>

#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>

#include <iostream>

#ifndef BASEDIR
#define BASEDIR "./"
#endif

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QTextEdit *editor = new QTextEdit;
    srchiliteqt::Qt4SyntaxHighlighter *highlighter =
            new srchiliteqt::Qt4SyntaxHighlighter(0);

    std::cout << "source-highlight data dir: " << srchilite::Settings::retrieveDataDir().c_str() << std::endl;

    if (argc > 1)
        highlighter->init(argv[1]);
    else {
        std::cout << "using " << BASEDIR "simple.lang" << std::endl;
        highlighter->init(BASEDIR "simple.lang");
    }

    highlighter->setDocument(editor->document());

    QMainWindow win(0);
    win.setCentralWidget(editor);

    win.setWindowTitle(QString("GNU Syntax Highlighter (using ") +
            QString(srchilite::Versions::getCompleteVersion().c_str()) + 
		       QString(")"));
    win.resize(700, 512);
    win.show();

    return app.exec();
}
