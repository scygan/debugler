/*
 * test_qt4_simple_main.cpp
 *
 * Simply creates an highlighter
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/Qt4SyntaxHighlighter.h>
#include <srchiliteqt/Qt4TextFormatter.h>
#include <iostream>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace srchilite;
using namespace std;

#undef main

int main() {
    Qt4SyntaxHighlighter *highlighter = new Qt4SyntaxHighlighter();

    delete highlighter;
    return 0;
}
