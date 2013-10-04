/*
 * test_qt3_syntaxhighlighter_main.cpp
 *
 * Check basic language definition file functionalities
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/Qt3SyntaxHighlighter.h>
#include <srchiliteqt/Qt3TextFormatter.h>
#include <iostream>
#include <qtextedit.h>
#include <qapplication.h>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace srchilite;
using namespace std;

static void printFormatter(const Qt3TextFormatter *qt4Formatter);

void printFormatter(const Qt3TextFormatter *qt3Formatter) {
    cout << qt3Formatter->getElem() << ": ";
    QColor &color = qt3Formatter->getQColor();
    QFont &font = qt3Formatter->getQFont();
    if (font.bold())
        cout << "BOLD ";
    if (font.italic())
        cout << "ITALIC ";
    if (font.underline())
        cout << "UNDERLINE ";
    cout << "foreground: " << color.name();
    //cout << " background: " << format.background().color().name().toStdString();
    cout << endl;
}

int main(int argc, char **argv) {
    // QApplication is not used, but it's needed for QTextEdit
    QApplication a( argc, argv );

    QTextEdit *edit = new QTextEdit;
    Qt3SyntaxHighlighter *highlighter = new Qt3SyntaxHighlighter(edit);

    highlighter->init("c.lang");

    FormatterManager *formatterManager = highlighter->getFormatterManager();

    Qt3TextFormatter *qt3Formatter =
            dynamic_cast<Qt3TextFormatter *> (formatterManager->getFormatter(
                    "keyword").get());

    assertTrue(qt3Formatter != 0);

    printFormatter(qt3Formatter);

    // check that they share the same formatters (meaning their QTextCharFormat
    QTextEdit *edit2 = new QTextEdit;
    Qt3SyntaxHighlighter *highlighter2 = new Qt3SyntaxHighlighter(edit2);
    highlighter2->init("java.lang");

    assertEquals(
            &(qt3Formatter->getQColor()),
            &(dynamic_cast<Qt3TextFormatter *> (highlighter2->getFormatterManager()->getFormatter(
                    "keyword").get())->getQColor()));

    assertEquals(
            &(qt3Formatter->getQFont()),
            &(dynamic_cast<Qt3TextFormatter *> (highlighter2->getFormatterManager()->getFormatter(
                    "keyword").get())->getQFont()));

    delete highlighter;
    delete highlighter2;
    delete edit;
    delete edit2;

    return 0;
}
