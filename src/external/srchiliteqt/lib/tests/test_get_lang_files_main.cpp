/*
 * test_get_lang_files_main.cpp
 *
 * Check basic language definition file functionalities
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qstring.h>

#include "srchiliteqt/GNUSyntaxHighlighter.h"
#include <srchilite/parserexception.h>
#include <srchilite/instances.h>
#include <iostream>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace srchilite;
using namespace std;

int main() {
    cout << "test_get_lang_files..." << endl;

    GNUSyntaxHighlighter syntaxHighlighter;

    // we must be able to find this lang def file
    HighlightStatePtr highlightState = syntaxHighlighter.getHighlightState(
            "java.lang");

    assertTrue(highlightState.get() != 0);

    highlightState = syntaxHighlighter.getHighlightState("c.lang");

    assertTrue(highlightState.get() != 0);

    // while this is non existent
    try {
        highlightState = syntaxHighlighter.getHighlightState("foobar.lang");
        assertFalse(true, "must not find foobar.lang");
    } catch (srchilite::ParserException &e) {
        cout << "expected exception: " << e << endl;
    }

    // check the initialization of the SourceHighlighter
    syntaxHighlighter.initHighlighter("perl.lang");

    highlightState = syntaxHighlighter.getHighlighter()->getCurrentState();
    assertTrue(highlightState.get() != 0);

    // while this is non existent
    try {
        syntaxHighlighter.initHighlighter("fooperl.lang");
        assertFalse(true, "must not find foobar.lang");
    } catch (srchilite::ParserException &e) {
        cout << "expected exception: " << e << endl;
    }

    // to avoid leaks, clean source-highlight global instances
    srchilite::Instances::unload();

    return 0;
}
