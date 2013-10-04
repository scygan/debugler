/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "MainColorForm.h"
#include "ui_MainColorForm.h"

#include <QColorDialog>

namespace srchiliteqt {

static QIcon createIcon(const QSize &size, const QColor &color) {
    QPixmap pixmap(size);
    pixmap.fill(color);
    return QIcon(pixmap);
}

MainColorForm::MainColorForm(QWidget *parent) :
    QWidget(parent), m_ui(new Ui::MainColorForm) {
    m_ui->setupUi(this);

    connect(m_ui->colorButton, SIGNAL(clicked()), this, SLOT(selectForeground()));
    connect(m_ui->backgroundColorButton, SIGNAL(clicked()), this, SLOT(selectBackground()));
}

MainColorForm::~MainColorForm() {
    delete m_ui;
}

void MainColorForm::changeEvent(QEvent *e) {
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainColorForm::setColor(const QColor &color) {
    if (color.isValid())
        m_ui->colorButton->setIcon(createIcon(m_ui->colorButton->iconSize(),
                color));
    foreground = color;
}

void MainColorForm::setBackgroundColor(const QColor &color) {
    if (color.isValid())
        m_ui->backgroundColorButton->setIcon(createIcon(
                m_ui->backgroundColorButton->iconSize(), color));
    background = color;
}

void MainColorForm::selectBackground()
{
    QColor color = QColorDialog::getColor(background);
    if (color.isValid()) {
        QToolButton *button = static_cast<QToolButton *>(sender());
        button->setIcon(createIcon(button->iconSize(), color));
        background = color;
    }
}

void MainColorForm::selectForeground()
{
    QColor color = QColorDialog::getColor(foreground);
    if (color.isValid()) {
        QToolButton *button = static_cast<QToolButton *>(sender());
        button->setIcon(createIcon(button->iconSize(), color));
        foreground = color;
    }
}

}
