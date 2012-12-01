/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef QT3TEXTFORMATTER_H_
#define QT3TEXTFORMATTER_H_

#include <qfont.h>
#include <qcolor.h>
#include <qsyntaxhighlighter.h>

#include <boost/shared_ptr.hpp>

#include "TextFormatter.h"
#include "Qt3SyntaxHighlighter.h"

namespace srchiliteqt {

typedef boost::shared_ptr<QFont> QFontPtr;
typedef boost::shared_ptr<QColor> QColorPtr;

/**
 * The implementation of TextFormatter for Qt3, relying on QTextCharFormat
 */
class Qt3TextFormatter: public TextFormatter {
protected:
    /**
     * The QFont for the formatter.  We use a shared pointer
     * so that on copy we still refer to the original object
     */
    QFontPtr font;

    /**
     * The QColor for the formatter.  We use a shared pointer
     * so that on copy we still refer to the original object
     */
    QColorPtr color;

    /**
     * The reference to QSyntaxHighlighter object.
     *
     * Since this header file does not include the header file for QSyntaxHighlighter,
     * then this same class can be re-used both for Qt3 and for Qt4 (the subclasses
     * of this class will include the correct header file for QSyntaxHighlighter).
     */
    Qt3SyntaxHighlighter *qSyntaxHighlighter;

public:
    Qt3TextFormatter(const std::string &elem_ = "normal");
    virtual ~Qt3TextFormatter();

    QFont &getQFont() const {
        return *font;
    }

    QColor &getQColor() const {
        return *color;
    }

    virtual void setQSyntaxHighlighter(QSyntaxHighlighter *qSyntaxHighlighter_) {
        qSyntaxHighlighter = dynamic_cast<Qt3SyntaxHighlighter *>(qSyntaxHighlighter_);
    }

    /**
     * Formats the passed string.
     *
     * @param the string to format
     * @param params possible additional parameters for the formatter (NOT USED)
     */
    void format(const std::string &s, const srchilite::FormatterParams *params);

};

}

#endif /* QT3TEXTFORMATTER_H_ */
