/*
 *  Copyright (C) 2008-2010  Lorenzo Bettini, http://www.lorenzobettini.it
 *  License: See COPYING file that comes with this distribution
 */

#ifndef MAINCOLORFORM_H
#define MAINCOLORFORM_H

#include <QtGui/QWidget>
#include <QColor>

namespace Ui {
    class MainColorForm;
}

namespace srchiliteqt {

/**
  * A form that can be used to select the color of the
  * main font of the editor
  */
class MainColorForm : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MainColorForm)
public:
    explicit MainColorForm(QWidget *parent = 0);
    virtual ~MainColorForm();

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
      * @return the color of the foreground (might be invalid)
      */
    const QColor &getColor() const {
        return foreground;
    }

    /**
      * @return the color of the background (might be invalid)
      */
    const QColor &getBackgroundColor() const {
        return background;
    }

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::MainColorForm *m_ui;

    QColor foreground;

    QColor background;

private slots:
    void selectBackground();
    void selectForeground();
};

}

#endif // MAINCOLORFORM_H
