#include "dglprogramview.h"
#include "dglgui.h"

#include "dglshaderviewitem.h"


DGLProgramViewItem::DGLProgramViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent), m_ResourceManager(resManager) {
    m_Ui.setupUi(this);

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeProgram);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLProgramViewItem::error(const std::string& message) {
    //TODO
}

void DGLProgramViewItem::update(const DGLResource& res) {

    const DGLResourceProgram* resource = dynamic_cast<const DGLResourceProgram*>(&res);
    std::string errorMsg;

    m_Ui.textEditLinker->setText(QString::fromStdString(resource->mLinkStatus.first));

    for (size_t i = 0; i <resource->m_AttachedShaders.size(); i++) {
        bool found = false;
        for (int j = 0; j < m_Ui.tabWidget->count(); j++) {
            DGLShaderViewItem* widget = dynamic_cast<DGLShaderViewItem*>(m_Ui.tabWidget->widget(j)); 
            if (widget && widget->getObjId() == resource->m_AttachedShaders[i].first) {
                found = true; break;
            }
        }
        if (!found) {
            DGLShaderViewItem* newTab = new DGLShaderViewItem(resource->m_AttachedShaders[i].first, m_ResourceManager, this);
            m_Ui.tabWidget->addTab(newTab, QString(GetShaderStageName(resource->m_AttachedShaders[i].second)
                + QString(" Shader ") + QString::number(resource->m_AttachedShaders[i].first)));
        }
    }
            
    if (!resource->mLinkStatus.second) {
        m_Ui.labelLinkStatus->setText(tr("Link status: failed"));
    } else {
        m_Ui.labelLinkStatus->setText(tr("Link status: success"));
    }
}

DGLProgramView::DGLProgramView(QWidget* parrent, DglController* controller):DGLTabbedView(parrent, controller) {
    setupNames("Shader Programs", "DGLProgramView");

    //inbound
    CONNASSERT(connect(controller->getViewRouter(), SIGNAL(showProgram(uint)), this, SLOT(showProgram(uint))));
}

void DGLProgramView::showProgram(uint name) {
    ensureTabDisplayed(name);
}

DGLTabbedViewItem* DGLProgramView::createTab(uint id) {
    return new DGLProgramViewItem(id, m_ResourceManager, this);
}

QString DGLProgramView::getTabName(uint id, uint target) {
    return QString("Program Shader ") + QString::number(id);
}