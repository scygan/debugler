/*
 * test_qt3_formatter_factory_main.cpp
 *
 * Check basic language definition file functionalities
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/Qt3TextFormatterFactory.h>
#include <srchiliteqt/Qt3TextFormatter.h>
#include <srchiliteqt/GNUSyntaxHighlighter.h>
#include <srchilite/parserexception.h>
#include <iostream>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace srchilite;
using namespace std;

static void printFormatter(const Qt3TextFormatter *qt3Formatter);

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

int main() {
    Qt3TextFormatterFactory formatterFactory;
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

    Qt3TextFormatter *qt3Formatter =
            dynamic_cast<Qt3TextFormatter *> (formatter.get());

    assertTrue(qt3Formatter != 0);

    assertTrue(qt3Formatter->getQFont().bold(),
            "not bold");
    assertTrue(!qt3Formatter->getQFont().italic(),
            "italic, but should not be");
    assertTrue(qt3Formatter->getQFont().underline(),
            "not underline");

    // now try with colors
    result = formatterFactory.createFormatter("foocolor", "green", "red",
            styleConstants);

    assertTrue(result);

    formatter = formatterFactory.getFormatter("foocolor");

    assertTrue(formatter.get() != 0);

    qt3Formatter = dynamic_cast<Qt3TextFormatter *> (formatter.get());

    assertTrue(qt3Formatter != 0);

    assertTrue(qt3Formatter->getQFont().bold(),
            "not bold");
    assertTrue(!qt3Formatter->getQFont().italic(),
            "italic, but should not be");
    assertTrue(qt3Formatter->getQFont().underline(),
            "not underline");

    cout << "green: " << TextFormatterFactory::colorMap.getColor("green")
            << endl;
    cout << "red: " << TextFormatterFactory::colorMap.getColor("red") << endl;

    assertEquals("#33CC00", TextFormatterFactory::colorMap.getColor("green"));
    assertEquals("#FF0000", TextFormatterFactory::colorMap.getColor("red"));

    QColor color(TextFormatterFactory::colorMap.getColor("green").c_str());

    cout << "green color: " << color.name() << endl;
    assertEquals("#33cc00", color.name());

    cout << "foreground: "
            << qt3Formatter->getQColor().name()
            << endl;
    //cout << "background: "
    //        << qt3Formatter->getQTextCharFormat().background().color().name().toStdString()
    //        << endl;

    assertEquals(
            "#33cc00",
            qt3Formatter->getQColor().name());
    //assertEquals(
    //        "#ff0000",
    //        qt3Formatter->getQTextCharFormat().background().color().name().toStdString());

    // now try to use the functionalities of GNUSyntaxHighlighter to read a style file
    GNUSyntaxHighlighter h;
    Qt3TextFormatterFactory formatterFactory2;

    // this will fill the formatter factory by reading the style file
    h.getTextFormatterMap(formatterFactory2);

    // the formatter for keyword and todo must be defined now
    qt3Formatter
            = dynamic_cast<Qt3TextFormatter *> (formatterFactory2.getFormatter(
                    "keyword").get());

    assertTrue(qt3Formatter != 0);
    printFormatter(qt3Formatter);

    qt3Formatter
            = dynamic_cast<Qt3TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get());

    assertTrue(qt3Formatter != 0);
    printFormatter(qt3Formatter);

    // check that the formatter factory is not rebuilt now on a second invocation
    h.getTextFormatterMap(formatterFactory2);

    assertEquals(qt3Formatter,
            dynamic_cast<Qt3TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get()));

    // check that the formatter factory is shared among the GNUSyntaxHighlighters
    GNUSyntaxHighlighter h2;
    h2.getTextFormatterMap(formatterFactory2);

    assertEquals(qt3Formatter,
            dynamic_cast<Qt3TextFormatter *> (formatterFactory2.getFormatter(
                    "todo").get()));

    return 0;
}
