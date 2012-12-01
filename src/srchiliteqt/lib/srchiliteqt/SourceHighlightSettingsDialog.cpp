/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include "SourceHighlightSettingsDialog.h"
#include "ui_SourceHighlightSettingsDialog.h"

namespace srchiliteqt {

SourceHighlightSettingsDialog::SourceHighlightSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SourceHighlightSettingsDialog)
{
    ui->setupUi(this);
}

SourceHighlightSettingsDialog::~SourceHighlightSettingsDialog()
{
    delete ui;
}

void SourceHighlightSettingsDialog::setSourceHighlightDataDirPath(const QString &path) {
    ui->sourceHighlightSettingsPage->setSourceHighlightDataDirPath(path);
}

const QString SourceHighlightSettingsDialog::getSourceHighlightDataDirPath() const {
    return ui->sourceHighlightSettingsPage->getSourceHighlightDataDirPath();
}

void SourceHighlightSettingsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

}
