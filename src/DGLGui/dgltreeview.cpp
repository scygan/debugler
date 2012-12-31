#include "dgltreeview.h"
#include "dglgui.h"

#include <set>
#include <climits>

DGLTreeView::DGLTreeView(QWidget* parrent, DglController* controller):QDockWidget(tr("State Tree"), parrent), m_TreeWidget(this),m_controller(controller) {
    setObjectName("DGLTreeView");

    m_TreeWidget.setMinimumSize(QSize(200, 0));

    setConnected(false);
    
    setWidget(&m_TreeWidget);
    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
    CONNASSERT(connect(controller, SIGNAL(debugeeInfo(const std::string&)), this, SLOT(debugeeInfo(const std::string&))));
    CONNASSERT(connect(controller, SIGNAL(setBreaked(bool)), &m_TreeWidget, SLOT(setEnabled(bool))));
    CONNASSERT(connect(controller, SIGNAL(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&)),
        this, SLOT(breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&))));
    
    //internal
    CONNASSERT(QObject::connect(&m_TreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        this, SLOT(onDoubleClicked(QTreeWidgetItem*, int))));
}


DGLTreeView::~DGLTreeView() {}

void DGLTreeView::setConnected(bool connected) {
    m_Connected = connected;
    if (!connected) {
        m_TreeWidget.setHeaderLabel("");
        while (m_TreeWidget.topLevelItemCount()) {
            delete m_TreeWidget.takeTopLevelItem(0);
        }
    }
}

void QClickableTreeWidgetItem::handleDoubleClick(DglController*) {}

DGLTextureWidget::DGLTextureWidget():m_name(0) {}

DGLTextureWidget::DGLTextureWidget(dglnet::ContextObjectNameTarget name):m_name(name) {
    setText(0, QString("Texture ") + QString::number(name.m_Name) + QString::fromStdString(" (" + GetTextureTargetName(name.m_Target) + ")"));
}

void DGLTextureWidget::handleDoubleClick(DglController* controller) {
    controller->getViewRouter()->show(m_name.m_Name, DGLResource::ObjectTypeTexture);
}

class DGLBufferWidget: public QClickableTreeWidgetItem {
public:
    DGLBufferWidget() {}
    DGLBufferWidget(dglnet::ContextObjectName name):m_name(name) {
        setText(0, QString("Buffer ") + QString::number(name.m_Name));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name.m_Name, DGLResource::ObjectTypeBuffer);
    }
private:
    dglnet::ContextObjectName m_name;
};

class DGLFBOWidget: public QClickableTreeWidgetItem {
public:
    DGLFBOWidget() {}
    DGLFBOWidget(dglnet::ContextObjectName name):m_name(name) {
        setText(0, QString("FBO ") + QString::number(name.m_Name));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name.m_Name, DGLResource::ObjectTypeFBO);
    }
private:
    dglnet::ContextObjectName m_name;
};

class DGLShaderWidget: public QClickableTreeWidgetItem {
public:
    DGLShaderWidget() {}
    DGLShaderWidget(dglnet::ContextObjectNameTarget name):m_name(name) {
        setText(0, QString("Shader ") + QString::number(name.m_Name) + QString::fromStdString(" (" + GetShaderStageName(name.m_Target) + ")"));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name.m_Name, DGLResource::ObjectTypeShader, m_name.m_Target);
    }
private:
    dglnet::ContextObjectNameTarget m_name;
};

class DGLProgramWidget: public QClickableTreeWidgetItem {
public:
    DGLProgramWidget() {}
    DGLProgramWidget(dglnet::ContextObjectName name):m_name(name) {
        setText(0, QString(" Shader Program ") + QString::number(name.m_Name));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name.m_Name, DGLResource::ObjectTypeProgram);
    }
private:
    dglnet::ContextObjectName m_name;
};


class DGLFramebufferWidget: public QClickableTreeWidgetItem {
public:
    DGLFramebufferWidget() {}
    DGLFramebufferWidget(dglnet::ContextObjectName type):m_type(type) {
        std::string name = "unknown";
        switch (type.m_Name) {
            case GL_FRONT_RIGHT:
                name = "Front right buffer"; break;
            case GL_BACK_RIGHT:
                name = "Back right buffer"; break;
            case GL_FRONT:
                name = "Front buffer"; break;
            case GL_BACK:
                name = "Back buffer"; break;
        }
        setText(0, QString(name.c_str()));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_type.m_Name, DGLResource::ObjectTypeFramebuffer);
    }
    dglnet::ContextObjectName m_type;
};

template<class ObjType>
class DGLObjectNodeWidget: public QClickableTreeWidgetItem {
public:
    DGLObjectNodeWidget(QString header) {
        setText(0, header);
    }
    template<typename T>
    void update(const std::set<T>& names) {
        for (std::set<T>::iterator i = names.begin(); i != names.end(); i++) {
            if (m_Childs.find(i->m_Name) == m_Childs.end()) {
                m_Childs[i->m_Name] = ObjType(*i);
                addChild(&m_Childs[i->m_Name]);
            }
        }
        std::map<uint, ObjType>::iterator i = m_Childs.begin();
        while (i != m_Childs.end()) {
            if (names.find(T(i->first)) == names.end()) {
                removeChild(&(i->second));
                i = m_Childs.erase(i);
            } else {
                i++;
            }
        }
    }
private:
    std::map<uint, ObjType> m_Childs;
};

class DGLCtxTreeWidget: public QClickableTreeWidgetItem  {
public:
    DGLCtxTreeWidget():m_TextureNode("Textures"), m_BufferNode("Vertex Buffers"), m_FBONode("Framebuffer objects"), m_ShaderNode("Shaders"), m_ProgramNode("Shader Programs"), m_FramebufferNode("Frame Buffers")  {
        addChild(&m_TextureNode);
        addChild(&m_BufferNode);
        addChild(&m_FBONode);
        addChild(&m_ShaderNode);
        addChild(&m_ProgramNode);
        addChild(&m_FramebufferNode);
    }
    uint getId() { return m_Id; }

    void update(const dglnet::ContextReport& report) {
        m_Id = report.m_Id;
        setText(0, QString("Context 0x") + QString::number(report.m_Id, 16));
        m_TextureNode.update(report.m_TextureSpace);
        m_BufferNode.update(report.m_BufferSpace);
        m_FBONode.update(report.m_FBOSpace);
        m_ShaderNode.update(report.m_ShaderSpace);
        m_ProgramNode.update(report.m_ProgramSpace);
        m_BufferNode.update(report.m_BufferSpace);
        m_FramebufferNode.update(report.m_FramebufferSpace);
    }

private:
    uint m_Id; 
    DGLObjectNodeWidget<DGLTextureWidget> m_TextureNode;
    DGLObjectNodeWidget<DGLBufferWidget> m_BufferNode;
    DGLObjectNodeWidget<DGLFBOWidget> m_FBONode;
    DGLObjectNodeWidget<DGLShaderWidget> m_ShaderNode;
    DGLObjectNodeWidget<DGLProgramWidget> m_ProgramNode;
    DGLObjectNodeWidget<DGLFramebufferWidget> m_FramebufferNode;
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

void DGLTreeView::debugeeInfo(const std::string& processName) {
    m_TreeWidget.setHeaderLabel(processName.c_str());
}