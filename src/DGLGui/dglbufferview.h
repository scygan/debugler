#ifndef DGLBUFFERVIEW_H
#define DGLBUFFERVIEW_H

#include "dgltabbedview.h"
#include "QHexEdit/qhexedit.h"

class DGLBufferView : public DGLTabbedView {
    Q_OBJECT

public:
    DGLBufferView(QWidget* parrent, DglController* controller);

    public slots:
        void showBuffer(uint);

private:
        virtual DGLTabbedViewItem* createTab(uint id);
        virtual QString getTabName(uint id, uint target);
};

class DGLBufferViewItem: public DGLTabbedViewItem {
    Q_OBJECT
public:
    DGLBufferViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent);

private slots:
    void error(const std::string& message);
    void update(const DGLResource& res);

private: 
    QHexEdit* m_Editor;
    QLabel* m_Label;
    QVBoxLayout* m_VerticalLayout;
    DGLResourceListener* m_Listener;
};

#endif //DGLBUFFERVIEW_H