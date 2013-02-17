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

class DGLTextureWidget: public QClickableTreeWidgetItem {
public:
    DGLTextureWidget() {}

    DGLTextureWidget(ContextObjectName name, QString iconPath):m_name(name) {
        setText(0, QString("Texture ") + QString::number(name.m_Name) + QString::fromStdString(" (" + GetTextureTargetName(name.m_Target) + ")"));
        setIcon(0, QIcon(iconPath));
    }

    virtual void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name, DGLResource::ObjectTypeTexture);
    }

private:
    ContextObjectName m_name;
};

class DGLBufferWidget: public QClickableTreeWidgetItem {
public:
    DGLBufferWidget() {}
    DGLBufferWidget(ContextObjectName name, QString iconPath):m_name(name) {
        setText(0, QString("Buffer ") + QString::number(name.m_Name));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name, DGLResource::ObjectTypeBuffer);
    }
private:
    ContextObjectName m_name;
};

class DGLFBOWidget: public QClickableTreeWidgetItem {
public:
    DGLFBOWidget() {}
    DGLFBOWidget(ContextObjectName name, QString iconPath):m_name(name) {
        setText(0, QString("FBO ") + QString::number(name.m_Name));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name, DGLResource::ObjectTypeFBO);
    }
private:
    ContextObjectName m_name;
};

class DGLShaderWidget: public QClickableTreeWidgetItem {
public:
    DGLShaderWidget() {}
    DGLShaderWidget(ContextObjectName name, QString iconPath):m_name(name) {
        setText(0, QString("Shader ") + QString::number(name.m_Name) + QString::fromStdString(" (" + GetShaderStageName(name.m_Target) + ")"));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name, DGLResource::ObjectTypeShader);
    }
private:
    ContextObjectName m_name;
};

class DGLProgramWidget: public QClickableTreeWidgetItem {
public:
    DGLProgramWidget() {}
    DGLProgramWidget(ContextObjectName name, QString iconPath):m_name(name) {
        setText(0, QString(" Shader Program ") + QString::number(name.m_Name));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name, DGLResource::ObjectTypeProgram);
    }
private:
    ContextObjectName m_name;
};


class DGLFramebufferWidget: public QClickableTreeWidgetItem {
public:
    DGLFramebufferWidget() {}
    DGLFramebufferWidget(ContextObjectName type, QString iconPath):m_type(type) {
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
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_type, DGLResource::ObjectTypeFramebuffer);
    }
    ContextObjectName m_type;
};

template<typename ObjType>
class DGLObjectNodeWidget: public QClickableTreeWidgetItem {
public:
    DGLObjectNodeWidget(uint ctxId, QString header, QString iconPath):m_CtxId(ctxId),m_IconPath(iconPath) {
        setText(0, header);
        setIcon(0, QIcon(iconPath));
    }
    template<typename T>
    void update(const std::set<T>& names) {
        typedef typename std::set<T>::iterator set_iter;
        for (set_iter i = names.begin(); i != names.end(); i++) {
            if (m_Childs.find(i->m_Name) == m_Childs.end()) {
                m_Childs[i->m_Name] = ObjType(*i, m_IconPath);
                addChild(&m_Childs[i->m_Name]);
            }
        }

        typedef typename std::map<uint, ObjType>::iterator map_iter;
        map_iter i = m_Childs.begin();
        while (i != m_Childs.end()) {
            map_iter next = i; next++;
            if (names.find(T(m_CtxId, i->first)) == names.end()) {
                removeChild(&(i->second));
                m_Childs.erase(i);
            }
            i = next;
        }
    }
private:
    std::map<uint, ObjType> m_Childs;
    uint m_CtxId;
    QString m_IconPath;
};

class DGLCtxTreeWidget: public QClickableTreeWidgetItem  {
public:
    DGLCtxTreeWidget(uint ctxId):m_Id(ctxId),
          m_TextureNode(ctxId, "Textures", ":/icons/texture.png"),
          m_BufferNode(ctxId, "Vertex Buffers", ":/icons/buffer.png"),
          m_FBONode(ctxId, "Framebuffer objects", ":/icons/fbo.png"),
          m_ShaderNode(ctxId, "Shaders", ":/icons/shader.png"),
          m_ProgramNode(ctxId, "Shader Programs", ":/icons/program.png"),
          m_FramebufferNode(ctxId, "Frame Buffers", ":/icons/framebuffer.png")  {
        addChild(&m_TextureNode);
        addChild(&m_BufferNode);
        addChild(&m_FBONode);
        addChild(&m_ShaderNode);
        addChild(&m_ProgramNode);
        addChild(&m_FramebufferNode);
        setIcon(0, QIcon(":/icons/context.png"));
    }
    uint getId() { return m_Id; }

    void update(const dglnet::ContextReport& report, bool current) {
        QFont fnt = font(0);
        fnt.setBold(current);
        setFont(0,fnt);
        assert(m_Id = report.m_Id);
        setText(0, QString("Context 0x") + QString::number(report.m_Id, 16) + (current?QString(" (current)"):QString("")));
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


void DGLTreeView::breakedWithStateReports(uint currentContextId, const std::vector<dglnet::ContextReport>& report) {
    for(size_t i = 0; i < report.size(); i++) {
        DGLCtxTreeWidget*  widget = 0; 
        for (uint j = 0; j < m_TreeWidget.topLevelItemCount(); j++) {
            DGLCtxTreeWidget*  thisWidget = dynamic_cast<DGLCtxTreeWidget*>(m_TreeWidget.topLevelItem(j)); 
            if ( thisWidget && thisWidget->getId() == report[i].m_Id) {
                widget = thisWidget; break;
            }
        }
        if (!widget) {
            widget = new DGLCtxTreeWidget(report[i].m_Id); 
            m_TreeWidget.addTopLevelItem(widget);
        }
        widget->update(report[i], report[i].m_Id == currentContextId);
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
