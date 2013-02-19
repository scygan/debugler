#ifndef DGLTEXTUREVIEW_H
#define DGLTEXTUREVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dgltextureviewitem.h"

class DGLTextureView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLTextureView(QWidget* parrent, DglController* controller);

public slots:
    void showTexture(uint ctx, uint name);

private:
    virtual DGLTabbedViewItem* createTab(const ContextObjectName& id);
    virtual QString getTabName(uint id, uint target);
};

class DGLTextureViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLTextureViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);
    void levelSliderMoved(int value);
    void faceComboChanged(int value);

private: 

    void internalUpdate();

    Ui::DGLTextureViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    std::vector<std::vector<boost::shared_ptr<DGLPixelRectangle> > > m_FacesLevels;

    DGLResourceListener* m_Listener;

    uint m_CurrentLevel, m_CurrentFace;
};

#endif // DGLTEXTUREVIEW_H
