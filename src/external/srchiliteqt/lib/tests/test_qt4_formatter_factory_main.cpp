/*
 * test_qt3_formatter_factory_main.cpp
 *
 * Check basic language definition file functionalities
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/Qt4TextFormatterFactory.h>
#include <srchiliteqt/Qt4TextFormatter.h>
#include <srchiliteqt/GNUSyntaxHighlighter.h>
#include <srchilite/parserexception.h>
#include <iostream>

#include "asserttestexit.h"

#undef main

using namespace srchiliteqt;
using namespace srchilite;
using namespace std;

static void printFormatter(const Qt4TextFormatter *qt4Formatter);

void printFormatter(const Qt4TextFormatter *qt4Formatter) {
    cout << qt4Formatter->getElem() << ": ";
    const QTextCharFormat &format = qt4Formatter->getQTextCharFormat();
    if (format.fontWeight() == QFont::Bold)
        cout << "BOLD ";
    if (format.fontItalic())
        cout << "ITALIC ";
    if (format.fontUnderline())
        cout << "UNDERLINE ";
    cout << "foreground: ";
    if (qt4Formatter->getForegroundColor().isValid())
        cout << format.foreground().color().name().toStdString();
    else
        cout << "INVALID";
    cout << " background: " << format.background().color().name().toStdString();
    cout << endl;
}

int main() {
    Qt4TextFormatterFactory formatterFactory;
    bool result = false;

    StyleConstantsPtr styleConstants(new StyleConstants);

    styleConstants->push_back(ISBOLD);
    styleConstants->push_back(ISUNDERLINE);

    // test without colors
    result = formatterFactory.createFormatter("foo", "", "", styleConstants);

    assertTrue(result);

    TextFormatterPtr formatter;

    formatter = formatterFactory.getFormatter("foo");

    assertTrue(formatter.get() != 0);

    Qt4TextFormatter *qt4Formatter =
            dynamic_cast<Qt4TextFormatter *> (formatter.get());

    assertTrue(qt4Formatter != 0);

    printFormatter(qt4Formatter);

    assertTrue(qt4Formatter->getQTextCharFormat().fontWeight() == QFont::Bold,
            "not bold");
    assertTrue(!qt4Formatter->getQTextCharFormat().fontItalic(),
            "italic, but should not be");
    assertTrue(qt4Formatter->getQTextCharFormat().fontUnderline(),
            "not underline");

    // now try with colors
    result = formatterFactory.createFormatter("foocolor", "green", "red",
            styleConstants);

    assertTrue(result);

    formatter = formatterFactory.getFormatter("foocolor");

    assertTrue(formatter.get() != 0);

    qt4Formatter = dynamic_cast<Qt4TextFormatter *> (formatter.get());

    assertTrue(qt4Formatter != 0);

    assertTrue(qt4Formatter->getQTextCharFormat().fontWeight() == QFont::Bold,
            "not bold");
    assertTrue(!qt4Formatter->getQTextCharFormat().fontItalic(),
            "italic, but should not be");
    assertTrue(qt4Formatter->getQTextCharFormat().fontUnderline(),
            "not underline");

    cout << "green: " << TextFormatterFactory::colorMap.getColor("green")
            << endl;
    cout << "red: " << TextFormatterFactory::colorMap.getColor("red") << endl;

    assertEquals("#33CC00", TextFormatterFactory::colorMap.getColor("green"));
    assertEquals("#FF0000", TextFormatterFactory::colorMap.getColor("red"));

    QColor color(TextFormatterFactory::colorMap.getColor("green").c_str());

    cout << "green color: " << color.name().toStdString() << endl;
    assertEquals("#33cc00", color.name().toStdString());

    cout << "foreground: "
            << qt4Formatter->getQTextCharFormat().foreground().color().name().toStdString()
            << endl;
    cout << "background: "
            << qt4Formatter->getQTextCharFormat().background().color().name().toStdString()
            << endl;

    assertEquals(
            "#33cc00",
            qt4Formatter->getQTextCharFormat().foreground().color().name().toStdString());
    assertEquals(
            "#ff0000",
            qt4Formatter->getQTextCharFormat().background().color().name().toStdString());

    // now try to use the functionalities of GNUSyntaxHighlighter to read a style file
    GNUSyntaxHighlighter h;
    Qt4TextFormatterFactory formatterFactory2;

    // this will fill the formatter factory by reading the style file
    h.getTextFormatterMap(formatterFactory2);

    // the formatter for keyword and todo must be defined now
    qt4Formatter
            = dynamic_cast<Qt4TextFormatter *> (formatterFactory2.getFormatter(
                    "keyword").get());

    assertTrue(qt4Formatter != 0);
    printFormatter(qt4Formatter);

    qt4Formatter
            = dynamic_cast<Qt4TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get());

    assertTrue(qt4Formatter != 0);
    printFormatter(qt4Formatter);

    // check that the formatter factory is not rebuilt now on a second invocation
    h.getTextFormatterMap(formatterFactory2);

    assertEquals(qt4Formatter,
            dynamic_cast<Qt4TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get()));

    // check that the formatter factory is shared among the GNUSyntaxHighlighters
    GNUSyntaxHighlighter h2;
    h2.getTextFormatterMap(formatterFactory2);

    assertEquals(qt4Formatter,
            dynamic_cast<Qt4TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get()));

    // test the translation into source-highlight style file format
    QString translated;
    formatterFactory.createFormatter("keyword", "blue", "", styleConstants);
    qt4Formatter
            = dynamic_cast<Qt4TextFormatter *> (formatterFactory.getFormatter(
                    "keyword").get());
    translated = qt4Formatter->toSourceHighlightStyleString();
    cout << "translated: " << translated.toStdString() << endl;
    assertEquals("keyword blue b, u, f;", translated.toStdString());

    formatterFactory.createFormatter("todo", "", "cyan", StyleConstantsPtr());
    qt4Formatter
            = dynamic_cast<Qt4TextFormatter *> (formatterFactory.getFormatter(
                    "todo").get());
    translated = qt4Formatter->toSourceHighlightStyleString();
    cout << "translated: " << translated.toStdString() << endl;
    assertEquals("todo bg:cyan f;", translated.toStdString());

    return 0;
}
