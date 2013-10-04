/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "ColorDialog.h"
#include "ui_ColorDialog.h"
#include "MainColorForm.h"
#include "LanguageElemColorForm.h"
#include "Qt4TextFormatter.h"

namespace srchiliteqt {

ColorDialog::ColorDialog(srchiliteqt::Qt4SyntaxHighlighter *highlighter_, QWidget *parent) :
    QDialog(parent), highlighter(highlighter_),
    m_ui(new Ui::ColorDialog)
{
    m_ui->setupUi(this);

    mainColorForm = new MainColorForm();
    mainColorForm->setColor(highlighter->getForegroundColor());
    mainColorForm->setBackgroundColor(highlighter->getBackgroundColor());

    addColorForm(mainColorForm);

    // now query the highlighter for all the text formatters, to fill
    // the color dialog
    srchiliteqt::Qt4TextFormatterMap formatterMap = highlighter->getQt4TextFormatterMap();
    srchiliteqt::Qt4TextFormatterMapIterator i(formatterMap);
    while (i.hasNext()) {
         i.next();
         LanguageElemColorForm *colorForm = new LanguageElemColorForm();
         colorForm->setColorDescription(i.key());
         colorForm->setColor(i.value()->getForegroundColor());
         colorForm->setBackgroundColor(i.value()->getBackgroundColor());
         colorForm->setBold(i.value()->isBold());
         colorForm->setItalic(i.value()->isItalic());
         colorForm->setUnderline(i.value()->isUnderline());
         colorForm->setMonospace(i.value()->isMonospace());

         addColorForm(colorForm);

         colorFormMap[i.key()] = colorForm;
    }
}

ColorDialog::~ColorDialog()
{
    delete m_ui;
}

void ColorDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ColorDialog::addColorForm(QWidget *form) {
    form->setParent(m_ui->area);
    m_ui->verticalLayout->addWidget(form);
}

void ColorDialog::syncFormatters() {
    // now query the highlighter for all the text formatters, to update
    // them with the data of the dialog
    srchiliteqt::Qt4TextFormatterMap formatterMap = highlighter->getQt4TextFormatterMap();
    srchiliteqt::Qt4TextFormatterMapIterator i(formatterMap);
    while (i.hasNext()) {
         i.next();
         LanguageElemColorForm *colorForm = colorFormMap[i.key()];
         if (colorForm) {
             srchiliteqt::Qt4TextFormatter *formatter = i.value();

             if (colorForm->getColor().isValid())
                formatter->setForegroundColor(colorForm->getColor());
             if (colorForm->getBackgroundColor().isValid())
                formatter->setBackgroundColor(colorForm->getBackgroundColor());
             formatter->setBold(colorForm->isBold());
             formatter->setItalic(colorForm->isItalic());
             formatter->setUnderline(colorForm->isUnderline());
             formatter->setMonospace(colorForm->isMonospace());
         }
    }

    // update foreground and background color of the highlighter
    QColor c = mainColorForm->getColor();
    if (c.isValid())
        highlighter->setForegroundColor(c.name());
    c = mainColorForm->getBackgroundColor();
    if (c.isValid())
        highlighter->setBackgroundColor(c.name());

}

}
