/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#include <QFileDialog>
#include <QDir>
#include <QDebug>

#include <srchilite/settings.h>

#include "SourceHighlightSettingsPage.h"
#include "ui_SourceHighlightSettingsPage.h"

namespace srchiliteqt {

SourceHighlightSettingsPage::SourceHighlightSettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SourceHighlightSettingsPage),
    sourceHighlightSettings(new srchilite::Settings)
{
    ui->setupUi(this);

    connect(ui->sourceHighlightDataDirLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validateDir(QString)));

    // defaults to current source-highlight settings
    QString currentPath = getSourceHighlightDataDirPath();
    if (currentPath.isEmpty()) {
        currentPath = srchilite::Settings::retrieveDataDir().c_str();
    }
    setSourceHighlightDataDirPath(currentPath);

    connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(browseDir()));
    connect(ui->defaultButton, SIGNAL(clicked()), this, SLOT(defaultDir()));

    validateDir(currentPath);
}

SourceHighlightSettingsPage::~SourceHighlightSettingsPage()
{
    delete ui;
    delete sourceHighlightSettings;
}

void SourceHighlightSettingsPage::setSourceHighlightDataDirPath(const QString &path) {
    ui->sourceHighlightDataDirLineEdit->setText(path);
}

const QString SourceHighlightSettingsPage::getSourceHighlightDataDirPath() const {
    return ui->sourceHighlightDataDirLineEdit->text();
}

void SourceHighlightSettingsPage::browseDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("Source-highlight data dir path"),
                                getSourceHighlightDataDirPath(),
                                QFileDialog::ShowDirsOnly);

     if (!directory.isEmpty()) {
         setSourceHighlightDataDirPath(directory);
     }

}

void SourceHighlightSettingsPage::defaultDir()
{
     setSourceHighlightDataDirPath(srchilite::Settings::getDefaultDataDir().c_str());
}

void SourceHighlightSettingsPage::validateDir(const QString &path) {
    sourceHighlightSettings->setDataDir(path.toStdString());

    if (sourceHighlightSettings->checkForTestFile()) {
        ui->errorLabel->setVisible(false);
        ui->okLabel->setVisible(true);
    } else {
        ui->errorLabel->setVisible(true);
        ui->okLabel->setVisible(false);
    }
}

void SourceHighlightSettingsPage::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

}
