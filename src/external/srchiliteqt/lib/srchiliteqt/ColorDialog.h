/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QtGui/QDialog>
#include <QMap>

#include "MainColorForm.h"
#include "Qt4SyntaxHighlighter.h"

namespace Ui {
    class ColorDialog;
}

namespace srchiliteqt {

class LanguageElemColorForm;

/**
  * A dialog to change the colors of the foreground and background
  * of an editor and of the font properties and colors of the
  * language elements to highlight.  The dialog automatically fills
  * the current properties by using the passed Qt4SyntaxHighlighter.
  * You can use the method syncFormatters() to update the formatters
  * of the highlighter with the value set in the dialog, e.g.,
  * @code
    ColorDialog dialog(textEdit->getHighlighter(), this);

    if (dialog.exec() == QDialog::Accepted) {
        dialog.syncFormatters();
        textEdit->getHighlighter()->rehighlight();

        // updating text editor colors is still up to us
        textEdit->changeColors(
                textEdit->getHighlighter()->getForegroundColor(),
                textEdit->getHighlighter()->getBackgroundColor());
    }
  * @endcode
  */
class ColorDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ColorDialog)

    /// the highlighter object
    srchiliteqt::Qt4SyntaxHighlighter *highlighter;
public:
    /**
      * @param the associated Qt4SyntaxHighlighter
      */
    explicit ColorDialog(srchiliteqt::Qt4SyntaxHighlighter *highlighter_, QWidget *parent = 0);
    virtual ~ColorDialog();

    /**
      * Addss a color form to this dialog
      * @param the color form to add
      */
    void addColorForm(QWidget *form);

    /**
      * Updates the text formatter of the highlighter using the
      * colors and font properties of the language element forms of this
      * dialog.  You should call this if the dialog is closed with
      * the accept button.
      */
    void syncFormatters();

protected:
    virtual void changeEvent(QEvent *e);

    MainColorForm *mainColorForm;

private:
    Ui::ColorDialog *m_ui;

    QMap<QString, LanguageElemColorForm *> colorFormMap;
};

}

#endif // COLORDIALOG_H
