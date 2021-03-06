/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "dgltreeview.h"

#include <set>
#include <climits>

DGLTreeView::DGLTreeView(QWidget* parrent, DglController* controller)
        : QDockWidget(tr("State Tree"), parrent),
          m_TreeWidget(this),
          m_controller(controller) {
    setObjectName("DGLTreeView");

    //    m_TreeWidget.setMinimumSize(QSize(200, 0));

    setConnected(false);

    setWidget(&m_TreeWidget);
    // inbound
    CONNASSERT(controller, SIGNAL(setConnected(bool)), this,
               SLOT(setConnected(bool)));
    CONNASSERT(controller, SIGNAL(debugeeInfo(const std::string&)), this,
               SLOT(debugeeInfo(const std::string&)));
    CONNASSERT(controller, SIGNAL(setBreaked(bool)), &m_TreeWidget,
               SLOT(setEnabled(bool)));
    CONNASSERT(controller,
               SIGNAL(breakedWithStateReports(
                       opaque_id_t,
                       const std::vector<
                               dglnet::message::utils::ContextReport>&)),
               this,
               SLOT(breakedWithStateReports(
                       opaque_id_t,
                       const std::vector<
                               dglnet::message::utils::ContextReport>&)));

    // internal
    CONNASSERT(&m_TreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
               this, SLOT(onDoubleClicked(QTreeWidgetItem*, int)));
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

const dglnet::ContextObjectName& QClickableTreeWidgetItem::getObjName() {
    return m_name;
}

void QClickableTreeWidgetItem::setObjName(const dglnet::ContextObjectName& name) {
    m_name = name;
}

class DGLTextureWidget : public QClickableTreeWidgetItem {
   public:
    DGLTextureWidget() {}

    DGLTextureWidget(QString iconPath) {
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("Texture ") + QString::number(name.m_Name) +
            QString::fromStdString(
            " (" + GetTextureTargetName(name.m_Target) +
            ")"));
        QClickableTreeWidgetItem::setObjName(name);
    }

    virtual void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Texture);
    }
};

class DGLProgramPipelineWidget : public QClickableTreeWidgetItem {
public:
    DGLProgramPipelineWidget() {}

    DGLProgramPipelineWidget(QString iconPath) {
            setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("Program ") + QString::number(name.m_Name));
        QClickableTreeWidgetItem::setObjName(name);
    }

    virtual void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
            m_name, dglnet::message::ObjectType::Texture);
    }
};


class DGLBufferWidget : public QClickableTreeWidgetItem {
   public:
    DGLBufferWidget() {}
    DGLBufferWidget(QString iconPath) {        
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("Buffer ") + QString::number(name.m_Name));
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Buffer);
    }
};

class DGLFBOWidget : public QClickableTreeWidgetItem {
   public:
    DGLFBOWidget() {}
    DGLFBOWidget(QString iconPath) {
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("FBO ") + QString::number(name.m_Name));
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name,
                                          dglnet::message::ObjectType::FBO);
    }
};

class DGLRenderbufferWidget : public QClickableTreeWidgetItem {
public:
    DGLRenderbufferWidget() {}
    DGLRenderbufferWidget(QString iconPath) {
            setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("Renderbuffer ") + QString::number(name.m_Name));
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name,
            dglnet::message::ObjectType::Renderbuffer);
    }
};

class DGLShaderWidget : public QClickableTreeWidgetItem {
   public:
    DGLShaderWidget() {}
    DGLShaderWidget(QString iconPath) {
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString("Shader ") + QString::number(name.m_Name) +
            QString::fromStdString(
            " (" + GetShaderStageName(name.m_Target) +
            ")"));
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Shader);
    }
};

class DGLProgramWidget : public QClickableTreeWidgetItem {
   public:
    DGLProgramWidget() {}
    DGLProgramWidget(QString iconPath) {
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        setText(0, QString(" Shader Program ") + QString::number(name.m_Name));
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Program);
    }
};

class DGLFramebufferWidget : public QClickableTreeWidgetItem {
   public:
    DGLFramebufferWidget() {}
    DGLFramebufferWidget(QString iconPath) {
        setIcon(0, QIcon(iconPath));
    }

    virtual void setObjName(const dglnet::ContextObjectName& name) {
        QString strname = "unknown";
        switch (name.m_Name) {
            case GL_FRONT_RIGHT:
                strname = "Front right buffer";
                break;
            case GL_BACK_RIGHT:
                strname = "Back right buffer";
                break;
            case GL_FRONT:
                strname = "Front buffer";
                break;
            case GL_BACK:
                strname = "Back buffer";
                break;
        }
        setText(0, strname);
        QClickableTreeWidgetItem::setObjName(name);
    }

    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Framebuffer);
    }
};

template <typename ObjType>
class DGLObjectNodeWidgetBase : public QTreeWidgetItem {
   public:
    DGLObjectNodeWidgetBase(opaque_id_t ctxId, QString header, QString iconPath)
            : m_IconPath(iconPath), m_CtxId(ctxId) {
        setHeader(header);
        setIcon(0, QIcon(iconPath));
    }

    void setHeader(QString header) {
        setText(0, header);
    }
    
   protected:
    QString m_IconPath;
    opaque_id_t m_CtxId;
};

template <typename ObjType>
class DGLObjectNodeWidget : public DGLObjectNodeWidgetBase<ObjType> {
public:
    DGLObjectNodeWidget(opaque_id_t ctxId, QString header, QString iconPath)
        : DGLObjectNodeWidgetBase<ObjType>(ctxId, header, iconPath) {}

    template <typename T>
    void update(const std::set<T>& names) {
        typedef typename std::set<T>::iterator set_iter;

        if (this->childCount() != (int)names.size()) {

            //remove excessive childs
            while (this->childCount() > (int)names.size()) {
                this->removeChild(this->child(this->childCount() - 1));
            }

            //add missing children
            while ((int)names.size() > this->childCount()) {
                this->addChild(new ObjType(this->m_IconPath));
            }
        }

        int childIdx = 0;
        for (set_iter i = names.begin(); i != names.end(); i++) {
            ObjType* typedChild = dynamic_cast<ObjType*>(this->child(childIdx));
            DGL_ASSERT(typedChild);
            if (typedChild) {
                if (!i->exactlySameAs(typedChild->getObjName())) {
                    typedChild->setObjName(*i);
                }
            }
            childIdx++;
        }
    }
};

template <typename ObjType>
class DGLIndexedObjectNodeWidget : public DGLObjectNodeWidgetBase<ObjType> {
public:
    DGLIndexedObjectNodeWidget(opaque_id_t ctxId, QString header, QString childHeader, QString iconPath)
        : DGLObjectNodeWidgetBase<ObjType>(ctxId, header, iconPath), m_ChildHeader(childHeader) {}
   
     template<typename T>
    void update(const std::vector<std::set<T> >& elements) {

        if (elements.size() != m_Childs.size()) {
            //resising a vector rellloc all pointers, 
            //so delete & create all object once again

            //this happend no more than once per ctx.

            for (size_t i = 0; i < m_Childs.size(); i++) {
                this->removeChild(&m_Childs[i]);
            }

            m_Childs.resize(elements.size(), DGLObjectNodeWidget<ObjType>(0, "", ""));

            for (size_t i = 0; i < m_Childs.size(); i++) {
                m_Childs[i] = 
                    DGLObjectNodeWidget<ObjType>(this->m_CtxId, m_ChildHeader + QString::number(i),
                            this->m_IconPath);
                this->addChild(&m_Childs[i]);
            }
        }

        DGL_ASSERT(elements.size() == m_Childs.size());
        
        for (size_t i = 0; i < elements.size(); i++) {
            m_Childs[i].update(elements[i]);
        }
    }
   private:
    std::vector<DGLObjectNodeWidget<ObjType> > m_Childs;
    QString m_ChildHeader;
};


template <typename ObjType>
class DGLAggregateObjectNodeWidget : public DGLObjectNodeWidgetBase<ObjType> {
public:
    DGLAggregateObjectNodeWidget(opaque_id_t ctxId, QString header, QString childHeader, QString iconPath)
        : DGLObjectNodeWidgetBase<ObjType>(ctxId, header, iconPath), m_ChildHeader(childHeader) {}

    template<typename T1, typename T2>
    void update(const std::set<std::pair<T1, std::set<T2> > >& elements) {

        if (elements.size() != m_Childs.size()) {
            //resising a vector rellloc all pointers, 
            //so delete & create all object once again

            //this happend no more than once per ctx.

            for (size_t i = 0; i < m_Childs.size(); i++) {
                this->removeChild(&m_Childs[i]);
            }

            m_Childs.resize(elements.size(), DGLObjectNodeWidget<ObjType>(this->m_CtxId, "", this->m_IconPath));

            for (size_t i = 0; i < m_Childs.size(); i++) {
                this->addChild(&m_Childs[i]);
            }

        }

        DGL_ASSERT(elements.size() == m_Childs.size());

        int idx = 0;
        for (typename std::set<std::pair<T1, std::set<T2> > >::iterator it = elements.begin(); it != elements.end(); it++) {

            //it: pointer to (object name, set<objects>) pairs;

            m_Childs[idx].setHeader(m_ChildHeader + QString::number(it->first.m_Name));

            m_Childs[idx].update(it->second);

            idx++;
        }

        for (size_t i = 0; i < elements.size(); i++) {
            
        }
    }
private:
    std::vector<DGLObjectNodeWidget<ObjType> > m_Childs;
    QString m_ChildHeader;
};

class DGLCtxTreeWidget : public QTreeWidgetItem {
   public:
    DGLCtxTreeWidget(opaque_id_t ctxId)
            : m_Id(ctxId),
              m_TextureNode(ctxId,         "Textures",             DGL_RES_ICON_TEXTURE_PATH),
              m_BufferNode(ctxId,          "Vertex Buffers",       DGL_RES_ICON_BUFFER_PATH),
              m_FBONode(ctxId,             "Framebuffer objects",  DGL_RES_ICON_FBO_PATH),
              m_RenderbufferNode(ctxId,    "Renderbuffer objects", DGL_RES_ICON_RENDERBUFFER_PATH),
              m_ShaderNode(ctxId,          "Shaders",              DGL_RES_ICON_SHADER_PATH),
              m_ProgramNode(ctxId,         "Shader Programs",      DGL_RES_ICON_PROGRAM_PATH),
              m_FramebufferNode(ctxId,     "Frame Buffers",        DGL_RES_ICON_FRAMEBUFFER_PATH),
              m_TextureUnitNode(ctxId,     "Texture units",    "Unit ",             DGL_RES_ICON_TEXTUREUNIT_PATH),
              m_ProgramPipelineNode(ctxId, "Pipeline objects", "Program Pipeline ", DGL_RES_ICON_PPO_PATH)  {
        addChild(&m_TextureNode);
        addChild(&m_BufferNode);
        addChild(&m_FBONode);
        addChild(&m_RenderbufferNode);
        addChild(&m_ShaderNode);
        addChild(&m_ProgramNode);
        addChild(&m_TextureUnitNode);
        addChild(&m_FramebufferNode);
        //addChild(&m_ProgramPipelineNode);
        setIcon(0, QIcon(":/icons/context.png"));
    }
    opaque_id_t getId() { return m_Id; }

    void update(const dglnet::message::utils::ContextReport& report,
                bool current) {
        QFont fnt = font(0);
        fnt.setBold(current);
        setFont(0, fnt);
        DGL_ASSERT(m_Id == report.m_Id);
        setText(0, QString("Context 0x") + QString::number(report.m_Id, 16) +
                           (current ? QString(" (current)") : QString("")));
        m_TextureNode.update(report.m_TextureSpace);
        m_BufferNode.update(report.m_BufferSpace);
        m_FBONode.update(report.m_FBOSpace);
        m_RenderbufferNode.update(report.m_RenderbufferSpace);
        m_ShaderNode.update(report.m_ShaderSpace);
        m_ProgramNode.update(report.m_ProgramSpace);
        m_BufferNode.update(report.m_BufferSpace);
        m_FramebufferNode.update(report.m_FramebufferSpace);
        m_TextureUnitNode.update(report.m_TextureUnitSpace);
        m_ProgramPipelineNode.update(report.m_ProgramPipelineSpace);
    }

   private:
    opaque_id_t m_Id;
    DGLObjectNodeWidget<DGLTextureWidget> m_TextureNode;
    DGLObjectNodeWidget<DGLBufferWidget> m_BufferNode;
    DGLObjectNodeWidget<DGLFBOWidget> m_FBONode;
    DGLObjectNodeWidget<DGLRenderbufferWidget> m_RenderbufferNode;
    DGLObjectNodeWidget<DGLShaderWidget> m_ShaderNode;
    DGLObjectNodeWidget<DGLProgramWidget> m_ProgramNode;
    DGLObjectNodeWidget<DGLFramebufferWidget> m_FramebufferNode;
    DGLIndexedObjectNodeWidget<DGLTextureWidget> m_TextureUnitNode;
    DGLAggregateObjectNodeWidget<DGLProgramWidget> m_ProgramPipelineNode;
};

void DGLTreeView::breakedWithStateReports(
        opaque_id_t currentContextId,
        const std::vector<dglnet::message::utils::ContextReport>&
                report) {
    for (size_t i = 0; i < report.size(); i++) {
        DGLCtxTreeWidget* treeWidget = 0;
        for (int j = 0; j < m_TreeWidget.topLevelItemCount(); j++) {
            DGLCtxTreeWidget* thisWidget = dynamic_cast<DGLCtxTreeWidget*>(
                    m_TreeWidget.topLevelItem(j));
            if (thisWidget && thisWidget->getId() == report[i].m_Id) {
                treeWidget = thisWidget;
                break;
            }
        }
        if (!treeWidget) {
            treeWidget = new DGLCtxTreeWidget(report[i].m_Id);
            m_TreeWidget.addTopLevelItem(treeWidget);
        }
        treeWidget->update(report[i], report[i].m_Id == currentContextId);
    }

    for (int j = 0; j < m_TreeWidget.topLevelItemCount(); j++) {
        DGLCtxTreeWidget* thisWidget =
                dynamic_cast<DGLCtxTreeWidget*>(m_TreeWidget.topLevelItem(j));
        bool found = false;
        for (size_t i = 0; i < report.size(); i++) {
            if (thisWidget && thisWidget->getId() == report[i].m_Id) {
                found = true;
                break;
            }
        }
        if (!found) {
            delete m_TreeWidget.takeTopLevelItem(j--);
        }
    }
}

void DGLTreeView::onDoubleClicked(QTreeWidgetItem* item, int) {
    QClickableTreeWidgetItem* clickableItem =
            dynamic_cast<QClickableTreeWidgetItem*>(item);
    if (clickableItem) {
        clickableItem->handleDoubleClick(m_controller);
    }
}

void DGLTreeView::debugeeInfo(const std::string& processName) {
    m_TreeWidget.setHeaderLabel(processName.c_str());
}
