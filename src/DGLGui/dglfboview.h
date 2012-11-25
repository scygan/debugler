#ifndef DGLFBOVIEW_H
#define DGLFBOVIEW_H

#include "dgltabbedview.h"


class DGLFBOView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFBOView(QWidget* parrent, DglController* controller);

    public slots:
        void showFBO(uint);
        void gotFBO(uint, const dglnet::FBOMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id);
};

#endif //DGLFRAMEBUFFERVIEW_H