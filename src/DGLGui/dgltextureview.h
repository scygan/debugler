#ifndef DGLTEXTUREVIEW_H
#define DGLTEXTUREVIEW_H

#include "dgltabbedview.h"

class DGLTextureView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLTextureView(QWidget* parrent, DglController* controller);

public slots:
    void showTexture(uint);
    void gotTexture(uint, const dglnet::TextureMessage&);

private:
    virtual DGLTabbedViewItem* createTab(uint id);
    virtual QString getTabName(uint id);
};

#endif // DGLTEXTUREVIEW_H