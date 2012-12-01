/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef SOURCEHIGHLIGHTSETTINGSPAGE_H
#define SOURCEHIGHLIGHTSETTINGSPAGE_H

#include <QWidget>

namespace Ui {
    class SourceHighlightSettingsPage;
}

namespace srchilite {
    class Settings;
}

namespace srchiliteqt {

/**
 * A page for a dialog for source-highlight's specific settings; this
 * particular page deals with the data dir value).
 *
 * The page also validates source-highlight data dir, showing
 * an error label, in case.
 *
 * @since 0.2
 */
class SourceHighlightSettingsPage : public QWidget {
    Q_OBJECT
public:
    SourceHighlightSettingsPage(QWidget *parent = 0);
    ~SourceHighlightSettingsPage();

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

protected slots:
    /**
      * Opens a dialog for selecting a directory
      */
    void browseDir();

    /**
     * Uses the default value for datadir (hardcoded path
     * in source-highlight library)
     */
    void defaultDir();

    /**
      * Checks that the specified path is a valid
      * source-highlight's data dir path.
      * @param path
      */
    void validateDir(const QString &path);

private:
    Ui::SourceHighlightSettingsPage *ui;

    srchilite::Settings *sourceHighlightSettings;
};

}

#endif // SOURCEHIGHLIGHTSETTINGSPAGE_H
