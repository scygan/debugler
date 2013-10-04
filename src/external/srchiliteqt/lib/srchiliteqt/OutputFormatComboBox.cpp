/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "OutputFormatComboBox.h"

#include <srchilite/sourcehighlightutils.h>

namespace srchiliteqt {

OutputFormatComboBox::OutputFormatComboBox(const QString &path) {
    setToolTip(tr("choose the output format for highlighting."));

    reload(path);
}

OutputFormatComboBox::OutputFormatComboBox(srchiliteqt::Qt4SyntaxHighlighter *highlighter)
{
    setToolTip(tr("choose the output format for highlighting."));

    std::set<std::string> langfiles = highlighter->getOutLangMap()->getMappedFileNames();
    for (std::set<std::string>::const_iterator it = langfiles.begin(); it != langfiles.end(); ++it) {
        addItem(QString((*it).c_str()));
    }
}

void OutputFormatComboBox::setCurrentOutputFormat(const QString &outlang) {
    int item = findText(outlang);
    if (item != -1)
        setCurrentIndex(item);
}

const QString OutputFormatComboBox::getCurrentOutputFormat() const {
    return currentText();
}

void OutputFormatComboBox::reload(const QString &path) {
    const QString &currentItem = getCurrentOutputFormat();

    // make sure we block signals when we reload the contents
    // otherwise we signal an empty current item
    bool prevState = blockSignals(true);

    clear();

    srchilite::StringSet files =
            srchilite::SourceHighlightUtils::getOutLangFileNames(path.toStdString());
    for (srchilite::StringSet::const_iterator it = files.begin(); it
            != files.end(); ++it) {
        addItem(QString((*it).c_str()));
    }

    // renable signals
    blockSignals(prevState);

    // and reselect the previous item (if it's still there)
    setCurrentOutputFormat(currentItem);

}

}
