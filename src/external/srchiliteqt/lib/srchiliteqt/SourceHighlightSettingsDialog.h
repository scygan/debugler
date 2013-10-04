/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef SOURCEHIGHLIGHTSETTINGSDIALOG_H
#define SOURCEHIGHLIGHTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
    class SourceHighlightSettingsDialog;
}

namespace srchiliteqt {

/**
 * A dialog for source-highlight's specific settings (for instance,
 * the data dir value).
 *
 * The dialog also validates source-highlight settings, showing
 * an error label, in case.
 *
 * @since 0.2
 */
class SourceHighlightSettingsDialog : public QDialog {
    Q_OBJECT
public:
    SourceHighlightSettingsDialog(QWidget *parent = 0);
    ~SourceHighlightSettingsDialog();

    /**
     * @param the value for the data-dir to show in the dialog
     */
    void setSourceHighlightDataDirPath(const QString &path);

    /**
     * @return the value of the data-dir of the dialog
     */
    const QString getSourceHighlightDataDirPath() const;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SourceHighlightSettingsDialog *ui;
};

}

#endif // SOURCEHIGHLIGHTSETTINGSDIALOG_H
