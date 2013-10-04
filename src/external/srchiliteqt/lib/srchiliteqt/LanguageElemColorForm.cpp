/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "LanguageElemColorForm.h"
#include "ui_LanguageElemColorForm.h"

#include <QColorDialog>
#include <QIcon>

namespace srchiliteqt {

static QIcon createIcon(const QSize &size, const QColor &color) {
    QPixmap pixmap(size);
    pixmap.fill(color);
    return QIcon(pixmap);
}

LanguageElemColorForm::LanguageElemColorForm(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::LanguageElemColorForm)
{
    m_ui->setupUi(this);

    connect(m_ui->colorButton, SIGNAL(clicked()), this, SLOT(selectForeground()));
    connect(m_ui->backgroundColorButton, SIGNAL(clicked()), this, SLOT(selectBackground()));
}

LanguageElemColorForm::~LanguageElemColorForm()
{
    delete m_ui;
}

void LanguageElemColorForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LanguageElemColorForm::setColorDescription(const QString &name) {
    m_ui->colorLabel->setText(name);
}

void LanguageElemColorForm::setColor(const QColor &color) {
    if (color.isValid())
        m_ui->colorButton->setIcon(createIcon(m_ui->colorButton->iconSize(),
                color));
    foreground = color;
}

void LanguageElemColorForm::setBackgroundColor(const QColor &color) {
    if (color.isValid())
        m_ui->backgroundColorButton->setIcon(createIcon(
                m_ui->backgroundColorButton->iconSize(), color));
    background = color;
}

bool LanguageElemColorForm::isBold() const {
    return m_ui->boldCheckBox->isChecked();
}

void LanguageElemColorForm::setBold(bool b) {
    m_ui->boldCheckBox->setChecked(b);
}

bool LanguageElemColorForm::isItalic() const {
    return m_ui->italicCheckBox->isChecked();
}

void LanguageElemColorForm::setItalic(bool i) {
    m_ui->italicCheckBox->setChecked(i);
}

bool LanguageElemColorForm::isUnderline() const {
    return m_ui->underlineCheckBox->isChecked();
}

void LanguageElemColorForm::setUnderline(bool u) {
    m_ui->underlineCheckBox->setChecked(u);
}

bool LanguageElemColorForm::isMonospace() const {
    return m_ui->monospaceCheckBox->isChecked();
}

void LanguageElemColorForm::setMonospace(bool m) {
    m_ui->monospaceCheckBox->setChecked(m);
}

void LanguageElemColorForm::selectBackground()
{
    QColor color = QColorDialog::getColor(background);
    if (color.isValid()) {
        QToolButton *button = static_cast<QToolButton *>(sender());
        button->setIcon(createIcon(button->iconSize(), color));
        background = color;
    }
}

void LanguageElemColorForm::selectForeground()
{
    QColor color = QColorDialog::getColor(foreground);
    if (color.isValid()) {
        QToolButton *button = static_cast<QToolButton *>(sender());
        button->setIcon(createIcon(button->iconSize(), color));
        foreground = color;
    }
}

}
