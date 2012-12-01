/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef OUTPUTFORMATCOMBOBOX_H
#define OUTPUTFORMATCOMBOBOX_H

#include <QComboBox>
#include "Qt4SyntaxHighlighter.h"

namespace srchiliteqt {

/**
  * A subclass of QComboBox which provides functionalities for showing
  * and selecting the output format definition files of Source-Highlight.
  *
  * @since 0.2
  */
class OutputFormatComboBox : public QComboBox
{
          Q_OBJECT
public:
     /**
       * Initializes the combo box with the .outlang files retrieved in the
       * specified path.
       * @param path where to look for .outlang files (if empty, uses the data dir
       * path of source-highlight)
       */
    OutputFormatComboBox(const QString &path = "");

    /**
     * @param _highlighter the highlighter object; this will be used only for initialization
     * @deprecated use the other constructor, which permits not needing a Qt4SyntaxHighlighter
     * only for initialization
     */
    OutputFormatComboBox(srchiliteqt::Qt4SyntaxHighlighter *_highlighter);

public slots:
    /**
     * Sets the output format in the combo
     * @param outlang the format file name; if the name is not part of the
     * combo list, nothing happens
     */
    void setCurrentOutputFormat(const QString &outlang);

    /**
      * @return the output format definition file name currently selected
      */
    const QString getCurrentOutputFormat() const;

    /**
     * Reloads the contents of the combo box by using the specified
     * path for searching for .outlang files.
     * @param path
     */
    void reload(const QString &path);

};

}

#endif // OUTPUTFORMATCOMBOBOX_H

