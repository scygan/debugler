/*
 * qt3_highlighter_example_main.cpp
 *
 *  Created on: Dec 2, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include "srchiliteqt/Qt3SyntaxHighlighter.h"

#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextedit.h>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    QMainWindow win(0);
    QTextEdit *editor = new QTextEdit(&win);
    srchiliteqt::Qt3SyntaxHighlighter *highlighter =
            new srchiliteqt::Qt3SyntaxHighlighter(editor);

    if (argc > 1)
        highlighter->init(argv[1]);
    else
        highlighter->init(BASEDIR "simple.lang");

    editor->setText("class Example");
    win.setCentralWidget(editor);
    win.setCaption("GNU Syntax Highlighter");
    win.resize(640, 512);
    win.show();

    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );

    return app.exec();
}

