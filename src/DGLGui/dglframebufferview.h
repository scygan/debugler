#ifndef DGLFRAMEBUFFERVIEW_H
#define DGLFRAMEBUFFERVIEW_H

#include "dgltabbedview.h"


class DGLFramebufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFramebufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showFramebuffer(uint);
        void gotFramebuffer(uint, const dglnet::FramebufferMessage&);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);
};

#endif //DGLFRAMEBUFFERVIEW_H