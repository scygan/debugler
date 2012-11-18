#ifndef DGLBUFFERVIEW_H
#define DGLBUFFERVIEW_H

#include "dgltabbedview.h"


class DGLBufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLBufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showBuffer(uint);
        void gotBuffer(uint, const dglnet::BufferMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id);
};

#endif //DGLBUFFERVIEW_H