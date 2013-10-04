/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "Qt3TextFormatterFactory.h"
#include "Qt3TextFormatter.h"

#include <qfont.h>
#include <qcolor.h>

using namespace srchilite;

namespace srchiliteqt {

Qt3TextFormatterFactory::Qt3TextFormatterFactory() {
}

Qt3TextFormatterFactory::~Qt3TextFormatterFactory() {
}

bool Qt3TextFormatterFactory::createFormatter(const string &key,
        const string &color, const string &bgcolor,
        srchilite::StyleConstantsPtr styleconstants) {

    if (hasFormatter(key))
        return false;

    Qt3TextFormatter *formatter = new Qt3TextFormatter(key);
    addFormatter(key, TextFormatterPtr(formatter));

    if (styleconstants.get()) {
        for (StyleConstantsIterator it = styleconstants->begin(); it
                != styleconstants->end(); ++it) {
            switch (*it) {
            case ISBOLD:
                formatter->getQFont().setBold(true);
                break;
            case ISITALIC:
                formatter->getQFont().setItalic(true);
                break;
            case ISUNDERLINE:
                formatter->getQFont().setUnderline(true);
                break;
            case ISFIXED:
                break;
            case ISNOTFIXED:
                // TODO
                break;
            case ISNOREF:
                break;
            }
        }
    }

    if (color.size()) {
        formatter->getQColor().setNamedColor(colorMap.getColor(color).c_str());
    }

    /*
     * TODO background color cannot be specified, can it?
    if (bgcolor.size()) {
        formatter->getQTextCharFormat().setBackground(QBrush(QColor(
                colorMap.getColor(bgcolor).c_str())));
    }
    */

    return true;
}

}
