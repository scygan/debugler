#ifndef DGLFBOVIEW_H
#define DGLFBOVIEW_H

#include "dgltabbedview.h"
#include "dglpixelrectangle.h"
#include "ui_dglfboviewitem.h"

class DGLFBOViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFBOViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource&);
    void showAttachment(int id);

private: 
    Ui_DGLFBOViewItem m_Ui;
    DGLPixelRectangleScene* m_PixelRectangleScene;
    boost::shared_ptr<DGLPixelRectangleView> m_PixelRectangleView;
    std::vector<DGLResourceFBO::FBOAttachment> m_Attachments;
    bool m_Error; 
    DGLResourceListener* m_Listener;
};



class DGLFBOView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLFBOView(QWidget* parrent, DglController* controller);

    public slots:
        void showFBO(uint);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);
};

#endif //DGLFRAMEBUFFERVIEW_H