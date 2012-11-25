#ifndef DGLFBOVIEW_H
#define DGLFBOVIEW_H

#include "dgltabbedview.h"
#include "ui_dglfboviewitem.h"

class DGLFBOViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLFBOViewItem(uint name, QWidget* parrent);

    void update(const dglnet::FBOMessage& msg);
private slots:

    void showAttachment(int id);

    virtual void requestUpdate(DglController* controller);
private: 
    Ui_DGLFBOViewItem m_Ui;
    boost::shared_ptr<QGraphicsScene> m_Scene;
    std::vector<dglnet::FBOAttachment> m_Attachments;
    bool m_Error; 
};



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