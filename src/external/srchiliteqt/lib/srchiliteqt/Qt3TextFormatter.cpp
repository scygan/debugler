/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "Qt3TextFormatter.h"

//#include <iostream>

namespace srchiliteqt {

Qt3TextFormatter::Qt3TextFormatter(const std::string &elem_) :
    TextFormatter(elem_), font(new QFont), color(new QColor) {
}

Qt3TextFormatter::~Qt3TextFormatter() {
}

void Qt3TextFormatter::format(const std::string &s,
        const srchilite::FormatterParams *params) {

//        std::cout << "formatting \"" << s << "\" as " << elem << std::endl;
        if (color->isValid())
            qSyntaxHighlighter->formatString(params->start, s.size(), *font,
                    *color);
        else
            qSyntaxHighlighter->formatString(params->start, s.size(), *font);
}

}
