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

class DGLTextureWidget : public QClickableTreeWidgetItem {
   public:
    DGLTextureWidget() {}

    DGLTextureWidget(dglnet::ContextObjectName name, QString iconPath)
            : m_name(name) {
        setText(0, QString("Texture ") + QString::number(name.m_Name) +
                           QString::fromStdString(
                                   " (" + GetTextureTargetName(name.m_Target) +
                                   ")"));
        setIcon(0, QIcon(iconPath));
    }

    virtual void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Texture);
    }

   private:
    dglnet::ContextObjectName m_name;
};

class DGLProgramPipelineWidget : public QClickableTreeWidgetItem {
public:
    DGLProgramPipelineWidget() {}

    DGLProgramPipelineWidget(dglnet::ContextObjectName name, QString iconPath)
        : m_name(name) {
            setText(0, QString("Program ") + QString::number(name.m_Name));
            setIcon(0, QIcon(iconPath));
    }

    virtual void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
            m_name, dglnet::message::ObjectType::Texture);
    }

private:
    dglnet::ContextObjectName m_name;
};


class DGLBufferWidget : public QClickableTreeWidgetItem {
   public:
    DGLBufferWidget() {}
    DGLBufferWidget(dglnet::ContextObjectName name, QString iconPath)
            : m_name(name) {
        setText(0, QString("Buffer ") + QString::number(name.m_Name));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Buffer);
    }

   private:
    dglnet::ContextObjectName m_name;
};

class DGLFBOWidget : public QClickableTreeWidgetItem {
   public:
    DGLFBOWidget() {}
    DGLFBOWidget(dglnet::ContextObjectName name, QString iconPath)
            : m_name(name) {
        setText(0, QString("FBO ") + QString::number(name.m_Name));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name,
                                          dglnet::message::ObjectType::FBO);
    }

   private:
    dglnet::ContextObjectName m_name;
};

class DGLRenderbufferWidget : public QClickableTreeWidgetItem {
public:
    DGLRenderbufferWidget() {}
    DGLRenderbufferWidget(dglnet::ContextObjectName name, QString iconPath)
        : m_name(name) {
            setText(0, QString("Renderbuffer ") + QString::number(name.m_Name));
            setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(m_name,
            dglnet::message::ObjectType::Renderbuffer);
    }

private:
    dglnet::ContextObjectName m_name;
};

class DGLShaderWidget : public QClickableTreeWidgetItem {
   public:
    DGLShaderWidget() {}
    DGLShaderWidget(dglnet::ContextObjectName name, QString iconPath)
            : m_name(name) {
        setText(0, QString("Shader ") + QString::number(name.m_Name) +
                           QString::fromStdString(
                                   " (" + GetShaderStageName(name.m_Target) +
                                   ")"));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Shader);
    }

   private:
    dglnet::ContextObjectName m_name;
};

class DGLProgramWidget : public QClickableTreeWidgetItem {
   public:
    DGLProgramWidget() {}
    DGLProgramWidget(dglnet::ContextObjectName name, QString iconPath)
            : m_name(name) {
        setText(0, QString(" Shader Program ") + QString::number(name.m_Name));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_name, dglnet::message::ObjectType::Program);
    }

   private:
    dglnet::ContextObjectName m_name;
};

class DGLFramebufferWidget : public QClickableTreeWidgetItem {
   public:
    DGLFramebufferWidget() {}
    DGLFramebufferWidget(dglnet::ContextObjectName objName, QString iconPath)
            : m_type(objName) {
        std::string name = "unknown";
        switch (objName.m_Name) {
            case GL_FRONT_RIGHT:
                name = "Front right buffer";
                break;
            case GL_BACK_RIGHT:
                name = "Back right buffer";
                break;
            case GL_FRONT:
                name = "Front buffer";
                break;
            case GL_BACK:
                name = "Back buffer";
                break;
        }
        setText(0, QString(name.c_str()));
        setIcon(0, QIcon(iconPath));
    }
    void handleDoubleClick(DglController* controller) {
        controller->getViewRouter()->show(
                m_type, dglnet::message::ObjectType::Framebuffer);
    }
    dglnet::ContextObjectName m_type;
};

template <typename ObjType>
class DGLObjectNodeWidgetBase : public QClickableTreeWidgetItem {
   public:
    DGLObjectNodeWidgetBase(opaque_id_t ctxId, QString header, QString iconPath)
            : m_CtxId(ctxId), m_IconPath(iconPath) {
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
        for (set_iter i = names.begin(); i != names.end(); i++) {
            if (m_Childs.find(i->m_Name) == m_Childs.end()) {
                m_Childs[i->m_Name] = ObjType(*i, DGLObjectNodeWidgetBase<ObjType>::m_IconPath);
                this->addChild(&m_Childs[i->m_Name]);
            }
        }

        typedef typename std::map<gl_t, ObjType>::iterator map_iter;
        map_iter i = m_Childs.begin();
        while (i != m_Childs.end()) {
            map_iter next = i;
            next++;
            if (names.find(T(DGLObjectNodeWidgetBase<ObjType>::m_CtxId, i->first)) == names.end()) {
                this->removeChild(&(i->second));
                m_Childs.erase(i);
            }
            i = next;
        }
    }
   private:
    std::map<gl_t, ObjType> m_Childs;
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
                    DGLObjectNodeWidget<ObjType>(DGLObjectNodeWidgetBase<ObjType>::m_CtxId, m_ChildHeader + QString::number(i),
                            DGLObjectNodeWidgetBase<ObjType>::m_IconPath);
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

            m_Childs.resize(elements.size(), DGLObjectNodeWidget<ObjType>(DGLObjectNodeWidgetBase<ObjType>::m_CtxId, "", DGLObjectNodeWidgetBase<ObjType>::m_IconPath));

            for (size_t i = 0; i < m_Childs.size(); i++) {
                this->addChild(&m_Childs[i]);
            }

        }

        DGL_ASSERT(elements.size() == m_Childs.size());

        int idx = 0;
        for (std::set<std::pair<T1, std::set<T2> > >::iterator it = elements.begin(); it != elements.end(); it++) {

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

class DGLCtxTreeWidget : public QClickableTreeWidgetItem {
   public:
    DGLCtxTreeWidget(opaque_id_t ctxId)
            : m_Id(ctxId),
              m_TextureNode(ctxId, "Textures", ":/icons/textures.png"),
              m_BufferNode(ctxId, "Vertex Buffers", ":/icons/buffer.png"),
              m_FBONode(ctxId, "Framebuffer objects", ":/icons/fbo.png"),
              m_RenderbufferNode(ctxId, "Renderbuffer objects", ":/icons/renderbuffer.png"),
              m_ShaderNode(ctxId, "Shaders", ":/icons/shader.png"),
              m_ProgramNode(ctxId, "Shader Programs", ":/icons/program.png"),
              m_FramebufferNode(ctxId, "Frame Buffers",
                                ":/icons/framebuffer.png"),
              m_TextureUnitNode(ctxId, "Texture units", "Unit ", ":/icons/textureunit.png"),
              m_ProgramPipelineNode(ctxId, "Pipeline objects", "Program Pipeline ", ":/icons/program_pipeline.png")  {
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
