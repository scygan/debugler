#include "dglprogramview.h"
#include "dglgui.h"

#include "dglshaderviewitem.h"


DGLProgramViewItem::DGLProgramViewItem(uint name, DGLResourceManager* resManager, QWidget* parrent):DGLTabbedViewItem(name, parrent), m_ResourceManager(resManager) {
    m_Ui.setupUi(this);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout_2->addWidget(m_Label);
    m_Ui.tableWidgetUniforms->setRowCount(1);
    m_Ui.tableWidgetUniforms->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(0, new QTableWidgetItem("name"));
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(1, new QTableWidgetItem("type"));
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(2, new QTableWidgetItem("value"));

    m_Listener = resManager->createListener(name, DGLResource::ObjectTypeProgram);
    m_Listener->setParent(this);

    CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
    CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
}

void DGLProgramViewItem::error(const std::string& message) {
    m_Ui.tabWidget->hide();
    m_Ui.groupBoxLinkStatus->hide();
    m_Ui.groupBoxUniforms->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
}

void DGLProgramViewItem::update(const DGLResource& res) {
    m_Ui.tabWidget->show();
    m_Ui.groupBoxLinkStatus->show();
    m_Ui.groupBoxUniforms->show();
    m_Label->hide();

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

    m_Ui.tableWidgetUniforms->setRowCount(resource->m_Uniforms.size());
    for (size_t i = 0; i < resource->m_Uniforms.size(); i++) { 
        QTableWidgetItem * item = new QTableWidgetItem(resource->m_Uniforms[i].m_name.c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(i, 0, item);

        item = new QTableWidgetItem(GetGLEnumName(resource->m_Uniforms[i].m_type).c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(i, 1, item);

        if (resource->m_Uniforms[i].m_supportedType) {
            std::stringstream valStream; 
            valStream << std::showpoint;
            for (int j = 0; j < resource->m_Uniforms[i].m_value.size(); j++) {
                if (j)
                    valStream << ", ";
                resource->m_Uniforms[i].m_value[j].writeToSS(valStream);
            }
            item = new QTableWidgetItem(valStream.str().c_str());
        } else {
            item = new QTableWidgetItem(tr("Not supported by debugger"));
        }
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(i, 2, item);
    }
    m_Ui.tableWidgetUniforms->resizeRowsToContents();

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