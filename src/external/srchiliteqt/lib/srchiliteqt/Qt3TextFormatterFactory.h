/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef QT3TEXTFORMATTERFACTORY_H_
#define QT3TEXTFORMATTERFACTORY_H_

#include "TextFormatterFactory.h"

namespace srchiliteqt {

/**
 * Implementation of TextFormatterFactory to create Qt3 TextFormatter objects
 * to format text in a TextEdit.
 */
class Qt3TextFormatterFactory: public TextFormatterFactory {
public:
    Qt3TextFormatterFactory();
    virtual ~Qt3TextFormatterFactory();

    /**
     * Creates a formatter for the specific language element (identified by
     * key) with the passed style parameters
     *
     * @param key
     * @param color
     * @param bgcolor
     * @param styleconstants
     * @return false if a formatter for the specific key is already present
     */
    virtual bool createFormatter(const string &key, const string &color,
            const string &bgcolor, srchilite::StyleConstantsPtr styleconstants);

};

}

#endif /* QT3TEXTFORMATTERFACTORY_H_ */
