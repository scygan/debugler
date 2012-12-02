#ifndef DGLFRAMEBUFFERVIEW_H
#define DGLFRAMEBUFFERVIEW_H

#include "dgltabbedview.h"
#include "ui_dglframebufferviewitem.h"

class DGLFramebufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFramebufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showFramebuffer(uint);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);
};

class DGLFramebufferViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFramebufferViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);

private: 
    Ui::DGLFramebufferViewItem m_Ui;
    boost::shared_ptr<QGraphicsScene> m_Scene;
    std::vector<uchar> m_PixelData;
    DGLResourceListener* m_Listener;
};

#endif //DGLFRAMEBUFFERVIEW_H