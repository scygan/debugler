/*
 * qt4_highlighter_view_main.cpp
 *
 * Differently from qt4_highlighter_edit_main.cpp, this takes as the
 * first command line argument the filename to show, and uses the
 * highlighting for the syntax detected from the filename itself
 * (e.g., it uses cpp.lang for foo.cpp, changelog.lang for ChangeLog, etc.).
 *
 * The text editor is in readonly mode, and also the highlight (this
 * should optimize the highlighting and make it faster).
 *
 *  Created on: Dec 2, 2008-2009
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include "srchiliteqt/Qt4SyntaxHighlighter.h"
#include "srchilite/versions.h"

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>

#include <iostream>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (argc <= 1) {
        std::cerr << "you must specify the file to edit" << std::endl;
        return 1;
    }

    QTextEdit *editor = new QTextEdit;
    editor->setReadOnly(true);
    srchiliteqt::Qt4SyntaxHighlighter *highlighter =
            new srchiliteqt::Qt4SyntaxHighlighter(editor->document());
    highlighter->setReadOnly(true);

    QMainWindow win(0);
    win.setCentralWidget(editor);

    QFile file(argv[1]);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << QString("Cannot read file %1:\n%2.") .arg(argv[1]) .arg(
                file.errorString()).toStdString() << std::endl;
        return 1;
    }

    if (!highlighter->initFromFileName(argv[1])) {
        std::cerr << "cannot find an highlighting scheme for " << argv[1]
                << std::endl;
        return 1;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    editor->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    win.setWindowTitle(QString("GNU Syntax Highlighter (using ") + QString(
            srchilite::Versions::getCompleteVersion().c_str()) + QString(")"));
    win.resize(700, 512);
    win.show();

    return app.exec();
}

