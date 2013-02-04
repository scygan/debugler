#ifndef DGLFRAMEBUFFERVIEW_H
#define DGLFRAMEBUFFERVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dglframebufferviewitem.h"

class DGLFramebufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFramebufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showFramebuffer(uint ctx, uint bufferEnum);

private:
        virtual DGLTabbedViewItem* createTab(const ContextObjectName& id);
        virtual QString getTabName(uint id, uint target);
};

class DGLFramebufferViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFramebufferViewItem(ContextObjectName name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);

private: 
    Ui::DGLFramebufferViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    boost::shared_ptr<DGLPixelRectangleView> m_PixelRectangleView;
    DGLResourceListener* m_Listener;
    boost::shared_ptr<DGLPixelRectangle> m_PixelRectangle;
};

#endif //DGLFRAMEBUFFERVIEW_H