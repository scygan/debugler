#include "dgltreeview.h"
#include "dglgui.h"

#include <set>
#include <climits>

DGLTreeView::DGLTreeView(QWidget* parrent, DglController* controller):QDockWidget(tr("State Tree"), parrent), m_TreeWidget(this),m_controller(controller) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    disable();
    
    setWidget(&m_TreeWidget);
    //inbound
    CONNASSERT(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    CONNASSERT(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    CONNASSERT(connect(controller, SIGNAL(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&)),
        this, SLOT(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&))));
    
    //internal
    CONNASSERT(QObject::connect(&m_TreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        this, SLOT(onDoubleClicked(QTreeWidgetItem*, int))));
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


void QClickableTreeWidgetItem::handleDoubleClick(DglController*) {}

DGLTextureWidget::DGLTextureWidget():m_name(0) {}

DGLTextureWidget::DGLTextureWidget(uint name):m_name(name) {
    setText(0, QString("Texture ") + QString::number(name));
}

void DGLTextureWidget::handleDoubleClick(DglController* controller) {
    controller->doShowTexture(m_name);
}

class DGLBufferWidget: public QClickableTreeWidgetItem {
public:
    DGLBufferWidget() {}
    DGLBufferWidget(uint name) {
        setText(0, QString("Buffer ") + QString::number(name));
    }
};

class DGLProgramWidget: public QClickableTreeWidgetItem {
public:
    DGLProgramWidget() {}
    DGLProgramWidget(uint name) {
        setText(0, QString("Program ") + QString::number(name));
    }
};

template<class ObjType>
class DGLObjectNodeWidget: public QClickableTreeWidgetItem {
public:
    DGLObjectNodeWidget(QString header) {
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

class DGLCtxTreeWidget: public QClickableTreeWidgetItem  {
public:
    DGLCtxTreeWidget():m_TextureNode("Textures"), m_BufferNode("Buffers"), m_ProgramNode("Programs")  {
        addChild(&m_TextureNode);
        addChild(&m_BufferNode);
        addChild(&m_ProgramNode);
    }
    uint getId() { return m_Id; }

    void update(const dglnet::ContextReport& report) {
        m_Id = report.m_Id;
        setText(0, QString("Context ") + QString::number(report.m_Id, 16));
        m_TextureNode.update(report.m_TextureSpace);
        m_BufferNode.update(report.m_BufferSpace);
        m_ProgramNode.update(report.m_ProgramSpace);
    }

private:
    uint m_Id; 
    DGLObjectNodeWidget<DGLTextureWidget> m_TextureNode;
    DGLObjectNodeWidget<DGLBufferWidget> m_BufferNode;
    DGLObjectNodeWidget<DGLProgramWidget> m_ProgramNode;
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


void DGLTreeView::onDoubleClicked(QTreeWidgetItem* item, int) {
    QClickableTreeWidgetItem* clickableItem = dynamic_cast<QClickableTreeWidgetItem*>(item);
    if (clickableItem) {
        clickableItem->handleDoubleClick(m_controller);
    }    
}