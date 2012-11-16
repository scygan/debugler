#ifndef DGLBUFFERVIEW_H
#define DGLBUFFERVIEW_H

#include <QDockWidget>
#include <QTabWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"


class DGLBufferView : public QDockWidget {
    Q_OBJECT

public:
    DGLBufferView(QWidget* parrent, DglController* controller);

    public slots:
        void enable();
        void disable();
        void showBuffer(uint);
        void gotBuffer(uint, dglnet::BufferMessage);

        private slots:
            void closeTab(int);

private: 
    QTabWidget m_TabWidget;
    bool m_Enabled;
    DglController* m_Controller;
};

#endif //DGLBUFFERVIEW_H