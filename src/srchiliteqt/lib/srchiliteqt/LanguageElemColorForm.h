/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef LANGUAGEELEMCOLORFORM_H
#define LANGUAGEELEMCOLORFORM_H

#include <QtGui/QWidget>

namespace Ui {
    class LanguageElemColorForm;
}

namespace srchiliteqt {

/**
  * A form that can be used to select the style of a language
  * element to highlight (for instance, color, bold, italic, etc.).
  */
class LanguageElemColorForm : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(LanguageElemColorForm)
public:
    explicit LanguageElemColorForm(QWidget *parent = 0);
    virtual ~LanguageElemColorForm();

    /**
      * Sets the color description label
      * @param name the color description label
      */
    void setColorDescription(const QString &name);

    /**
      * Sets the color of the foreground button (if valid)
      * @param color the color of the button
      */
    void setColor(const QColor &color);

    /**
      * Sets the color of the background button (if valid)
      * @param color the color of the button
      */
    void setBackgroundColor(const QColor &color);

    /**
      * @return the color of the foreground
      */
    const QColor &getColor() const {
        return foreground;
    }

    /**
      * @return the color of the background
      */
    const QColor &getBackgroundColor() const {
        return background;
    }

    bool isBold() const;

    void setBold(bool b);

    bool isItalic() const;

    void setItalic(bool i);

    bool isUnderline() const;

    void setUnderline(bool u);

    bool isMonospace() const;

    void setMonospace(bool m);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::LanguageElemColorForm *m_ui;

    QColor foreground;

    QColor background;

private slots:
    void selectBackground();
    void selectForeground();
};

}

#endif // LANGUAGEELEMCOLORFORM_H
