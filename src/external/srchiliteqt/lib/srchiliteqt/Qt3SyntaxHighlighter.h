/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef QT3SYNTAXHIGHLIGHTER_H_
#define QT3SYNTAXHIGHLIGHTER_H_

#include <qsyntaxhighlighter.h>
#include <qtextedit.h>
#include <qfont.h>
#include <qcolor.h>

#include "GNUSyntaxHighlighter.h"
#include "ParagraphMap.h"

namespace srchiliteqt {

/**
 * An implementation of QSyntaxHighlighter using GNU Source-highlight library
 * (by relying on GNUSyntaxHighlighter provided by the common part of this library).
 *
 * You can use such highlighter with a QTextEdit, and initialize the highlighter
 * with the language definition file, e.g.,
 * @code
    QTextEdit *editor = new QTextEdit;
    srchiliteqt::Qt3SyntaxHighlighter *highlighter =
            new srchiliteqt::Qt3SyntaxHighlighter(editor->document());
    highlighter->init("java.lang");
 * @endcode
 */
class Qt3SyntaxHighlighter: public QSyntaxHighlighter,
        public GNUSyntaxHighlighter {
protected:
    /// used internally to associate HighlightStateData to paragraphs
    ParagraphMap paragraphMap;

    int highlightParagraph(const QString & text, int endStateOfLastPara);

public:
    Qt3SyntaxHighlighter(QTextEdit *parent = 0);
    virtual ~Qt3SyntaxHighlighter();

    /**
     * Initializes this highlighter with the specified language definition file
     * @param langFile
     */
    void init(const std::string &langFile);

    /**
     * This function is applied to the syntax highlighter's current text block
     * (i.e. the text that is passed to the highlightParagraph() method).
     *
     * The specified font and color are applied to the text from the start position
     * for a length of count characters
     * (if count is 0, nothing is done).
     * The formatting properties set in format are merged at display
     * time with the formatting information stored directly in the document,
     * for example as previously set with QTextCursor's functions.
     *
     * Note that this helper function will be called by the corresponding
     * TextFormatter, from Source-highglight library code, and relies on
     * the corresponding protected method of QSyntaxHighlighter: setFormat).
     */
    void formatString(int start, int count, const QFont &font,
            const QColor &color) {
        setFormat(start, count, font, color);
    }

    void formatString(int start, int count, const QFont &font) {
        setFormat(start, count, font);
    }

};

}

#endif /* QT3SYNTAXHIGHLIGHTER_H_ */
