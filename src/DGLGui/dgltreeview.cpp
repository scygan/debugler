#include "dgltreeview.h"

#include <set>
#include <climits>

DGLTreeView::DGLTreeView(QWidget* parrent, DglController* controller):QDockWidget(tr("State Tree"), parrent), m_TreeWidget(this) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    disable();
    
    setWidget(&m_TreeWidget);
    //inbound
    assert(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    assert(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    assert(connect(controller, SIGNAL(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&)),
        this, SLOT(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&))));
    
}


DGLTreeView::~DGLTreeView() {}

void DGLTreeView::enable() {
    m_Enabled = true;
}

void DGLTreeView::disable() {
    m_Enabled = false;
    while (m_TreeWidget.topLevelItemCount()) {
        delete m_TreeWidget.takeTopLevelItem(0);
    }
}


class DGLTextureWidget: public QTreeWidgetItem {
public:
    DGLTextureWidget() {}
    DGLTextureWidget(uint name) {
        setText(0, QString("Texture ") + QString::number(name));
    }
};

class DGLBufferWidget: public QTreeWidgetItem {
public:
    DGLBufferWidget() {}
    DGLBufferWidget(uint name) {
        setText(0, QString("Buffer ") + QString::number(name));
    }
};

template<class ObjType>
class DGLObjectSpaceWidget: public QTreeWidgetItem {
public:
    DGLObjectSpaceWidget(QString header) {
        setText(0, header);
    }
    void update(const std::set<uint32_t>& names) {
        for (std::set<uint32_t>::iterator i = names.begin(); i != names.end(); i++) {
            if (m_Childs.find(*i) == m_Childs.end()) {
                m_Childs[*i] = ObjType(*i);
                addChild(&m_Childs[*i]);
            }
        }
        for (std::map<uint, ObjType>::iterator i = m_Childs.begin(); i != m_Childs.end(); i++) {
            if (names.find(i->first) == names.end()) {
                removeChild(&(i->second));
                m_Childs.erase(i);
            }
        }
    }
private:
    std::map<uint, ObjType> m_Childs;
};

class DGLCtxTreeWidget: public QTreeWidgetItem  {
public:
    DGLCtxTreeWidget():m_TextureSpace("Textures"),m_BufferSpace("Buffers")  {
        addChild(&m_TextureSpace);
        addChild(&m_BufferSpace);
    }
    uint getId() { return m_Id; }

    void update(const dglnet::ContextReport& report) {
        m_Id = report.m_Id;
        setText(0, QString("Context ") + QString::number(report.m_Id, 16));
        m_TextureSpace.update(report.m_TextureSpace);
        m_BufferSpace.update(report.m_BufferSpace);
    }

private:
    uint m_Id; 
    DGLObjectSpaceWidget<DGLTextureWidget> m_TextureSpace;
    DGLObjectSpaceWidget<DGLBufferWidget> m_BufferSpace;
};


void DGLTreeView::breakedWithStateReports(uint ctxID, const std::vector<dglnet::ContextReport>& report) {
    for(size_t i = 0; i < report.size(); i++) {
        DGLCtxTreeWidget*  widget = 0; 
        for (uint j = 0; j < m_TreeWidget.topLevelItemCount(); j++) {
            DGLCtxTreeWidget*  thisWidget = dynamic_cast<DGLCtxTreeWidget*>(m_TreeWidget.topLevelItem(j)); 
            if ( thisWidget && thisWidget->getId() == report[i].m_Id) {
                widget = thisWidget; break;
            }
        }
        if (!widget) {
            widget = new DGLCtxTreeWidget(); 
            m_TreeWidget.addTopLevelItem(widget);
        }
        widget->update(report[i]);
    }    
    
    for (uint j = 0; j < m_TreeWidget.topLevelItemCount(); j++) {
        DGLCtxTreeWidget* thisWidget = dynamic_cast<DGLCtxTreeWidget*>(m_TreeWidget.topLevelItem(j)); 
        bool found = false;
        for(size_t i = 0; i < report.size(); i++) {
            if ( thisWidget && thisWidget->getId() == report[i].m_Id) {
                found = true; break;
            }
        }
        if (!found) {
            delete m_TreeWidget.takeTopLevelItem(j--);
        }
    }
}