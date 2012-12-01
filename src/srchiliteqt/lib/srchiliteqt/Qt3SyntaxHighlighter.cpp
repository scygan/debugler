/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "Qt3TextFormatter.h"
#include "Qt3TextFormatterFactory.h"

#include "Qt3SyntaxHighlighter.h"

//#define SHOW_DEBUG_INFO 1

#ifdef SHOW_DEBUG_INFO
#include <iostream>
#define DEBUG_INFO(x,y) std::cout << x << y << std::endl;
#else
#define DEBUG_INFO(x,y)
#endif

namespace srchiliteqt {

/// the shared TextFormatterFactory
static Qt3TextFormatterFactory factory;

Qt3SyntaxHighlighter::Qt3SyntaxHighlighter(QTextEdit *parent) :
    QSyntaxHighlighter(parent) {

}

Qt3SyntaxHighlighter::~Qt3SyntaxHighlighter() {
}

void Qt3SyntaxHighlighter::init(const std::string &langFile_) {
    if (!getFormatterManager()) {
        TextFormatterPtr defaultFormatter = TextFormatterPtr(
                new Qt3TextFormatter("normal"));
        formatterManager = new srchilite::FormatterManager(defaultFormatter);

        defaultFormatter->setQSyntaxHighlighter(this);

        const TextFormatterMap &formatterMap = getTextFormatterMap(factory);

        /*
         * For each element in the TextFormatterMap, we need to create a copy
         * (that will share the crucial formatting things) and set this QSyntaxHighlighter
         * pointer (the formatters will call setFormat on such pointer).
         */
        for (TextFormatterMap::const_iterator it = formatterMap.begin(); it
                != formatterMap.end(); ++it) {
            Qt3TextFormatter *formatter =
                    dynamic_cast<Qt3TextFormatter *> (it->second.get());
            Qt3TextFormatter *copiedFormatter =
                    new Qt3TextFormatter(*formatter);
            copiedFormatter->setQSyntaxHighlighter(this);
            formatterManager->addFormatter(it->first, TextFormatterPtr(
                    copiedFormatter));
        }
    }

    initHighlighter(langFile_);
}

int Qt3SyntaxHighlighter::highlightParagraph(const QString & text,
        int endStateOfLastPara) {
    DEBUG_INFO("", "");
    DEBUG_INFO("highlightBlock: ", "\"" + text + "\"");
    DEBUG_INFO("prevBlockState: ", endStateOfLastPara);

    int currentPar = currentParagraph();

    DEBUG_INFO("current paragraph: ", currentPar);

    HighlightStateData *prevBlockData = 0;
    if (currentPar > 0) {
        // retrieve the data of the previous paragraph
        prevBlockData = paragraphMap.getData(currentPar - 1);
    }

    // retrieve the current state of the current paragraph
    DEBUG_INFO("currentBlockState: ", (currentPar >= 0 && paragraphMap.getData(currentPar) ?
                paragraphMap.getData(currentPar)->currentState->getId() : 0));

    HighlightStateData *blockData = 0;

    /*
     * The current block must use the highlighting state of the previous
     * QTextBlock (actually a copy of it)
     */
    if (prevBlockData) {
        // make a copy
        blockData = new HighlightStateData(*(prevBlockData));
    }

    // does the actual highlighting
    highlightLine(text, blockData);

    int blockState = 0;
    if (blockData) {
        // we changed the highlighting state
        blockState = blockData->currentState->getId();
    }

    // we record this data for the current paragraph
    paragraphMap.insert(currentPar, blockData);

    DEBUG_INFO("currentBlockState: ", blockState);

    // this is crucial for QSyntaxHighlighter to know whether other parts
    // of the document must be re-highlighted
    return blockState;
}

}
