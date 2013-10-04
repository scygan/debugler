/*
 * test_colormap_main.cpp
 *
 * Check color map
 *
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qstring.h>

#include "srchiliteqt/QtColorMap.h"
#include <iostream>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace std;

int main() {
    cout << "test_colormap..." << endl;

    QtColorMap colorMap;
    QtColorMapRGB colorMapRGB;

    // first simple check
    assertEquals("darkblue", colorMapRGB.getColor("#000080"));

    // check that the mapping is bidirectional
    for (QtColorMap::const_iterator it = colorMap.begin(); it != colorMap.end(); ++it) {
        assertEquals(colorMapRGB.getColor(it->second), it->first);
    }

    // check for non existing color in QtColorMapRGB (it must return the
    // passed rgb string in this case)
    assertEquals("#EEEEEE", colorMapRGB.getColor("#EEEEEE"));

    // check for case insensitive
    assertEquals("yellow", colorMapRGB.getColor("#FFCC00"));
    assertEquals("yellow", colorMapRGB.getColor("#ffcc00"));

    return 0;
}
