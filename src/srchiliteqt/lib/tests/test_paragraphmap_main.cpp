/*
 * test_qt3_formatter_factory_main.cpp
 *
 * Check basic language definition file functionalities
 *
 *  Created on: Dec 3, 2008
 *      Author: Lorenzo Bettini <http://www.lorenzobettini.it>, (C) 2008
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <srchiliteqt/ParagraphMap.h>
#include <iostream>

#include "asserttestexit.h"

using namespace srchiliteqt;
using namespace std;

int main() {
    ParagraphMap paragraphMap;

    // check that initially all data pointers are 0
    for (int i = 0; i < STARTING_SIZE; ++i) {
        assertTrue(paragraphMap.getData(i) == 0, "data is not 0");
    }

    // try to insert and retrieve it
    HighlightStateData *data = new HighlightStateData();
    paragraphMap.insert(50, data);
    assertEquals(data, paragraphMap.getData(50));

    // try to insert at position STARTING_SIZE (i.e., one past the end)
    HighlightStateData *data1 = new HighlightStateData();
    HighlightStateData *data2 = new HighlightStateData();
    paragraphMap.insert(STARTING_SIZE, data1);
    paragraphMap.insert(STARTING_SIZE+1, data2);
    assertEquals(data1, paragraphMap.getData(STARTING_SIZE));
    assertEquals(data2, paragraphMap.getData(STARTING_SIZE+1));

    return 0;
}
