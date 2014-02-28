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

#include "dglprogramview.h"

#include "dglshaderviewitem.h"

#include <DGLNet/protocol/request.h>
//#include <DGLNet/protocol/message.h>

#include <QMessageBox>
#include <sstream>

DGLProgramViewItem::DGLProgramViewItem(dglnet::ContextObjectName name,
                                       DGLResourceManager* resManager,
                                       QWidget* parrent)
        : DGLTabbedViewItem(name, parrent),
          DGLRequestHandler(resManager->getRequestManager()),
          m_RequestManager(resManager->getRequestManager()),
          m_ResourceManager(resManager),
          m_Name(name) {
    m_Ui.setupUi(this);

    m_Label = new QLabel(this);
    m_Ui.verticalLayout_2->addWidget(m_Label);
    m_Ui.tableWidgetUniforms->setRowCount(1);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    m_Ui.tableWidgetUniforms->horizontalHeader()->setResizeMode(
            QHeaderView::ResizeToContents);
#else
    m_Ui.tableWidgetUniforms->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
#endif
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(
            kUniformTable_NameIdx, new QTableWidgetItem("name"));
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(
            kUniformTable_TypeIdx, new QTableWidgetItem("type"));
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(
            kUniformTable_ValueIdx, new QTableWidgetItem("value"));
    m_Ui.tableWidgetUniforms->setHorizontalHeaderItem(
            kUniformTable_LocationIdx, new QTableWidgetItem("location"));

    m_Listener = resManager->createListener(
            name, dglnet::message::ObjectType::Program);
    m_Listener->setParent(this);

    CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
               SLOT(update(const dglnet::DGLResource&)));
    CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
               SLOT(error(const std::string&)));
}

void DGLProgramViewItem::error(const std::string& message) {
    m_Ui.tabWidget->hide();
    m_Ui.groupBoxLinkStatus->hide();
    m_Ui.groupBoxUniforms->hide();
    m_Label->setText(QString::fromStdString(message));
    m_Label->show();
}

void DGLProgramViewItem::update(const dglnet::DGLResource& res) {
    m_Ui.tabWidget->show();
    m_Ui.groupBoxLinkStatus->show();
    m_Ui.groupBoxUniforms->show();
    m_Label->hide();

    const dglnet::resource::DGLResourceProgram* resource =
            dynamic_cast<const dglnet::resource::DGLResourceProgram*>(&res);

    m_Ui.textEditLinker->setText(
            QString::fromStdString(resource->mLinkStatus.first));

    for (size_t i = 0; i < resource->m_AttachedShaders.size(); i++) {
        bool found = false;
        for (int j = 0; j < m_Ui.tabWidget->count(); j++) {
            DGLShaderViewItem* widget =
                    dynamic_cast<DGLShaderViewItem*>(m_Ui.tabWidget->widget(j));
            if (widget && widget->getObjName().m_Name ==
                                  resource->m_AttachedShaders[i].first) {
                found = true;
                break;
            }
        }
        if (!found) {
            DGLShaderViewItem* newTab = new DGLShaderViewItem(
                    dglnet::ContextObjectName(
                            getObjName().m_Context,
                            resource->m_AttachedShaders[i].first),
                    m_ResourceManager, this);
            m_Ui.tabWidget->addTab(
                    newTab,
                    QString::fromStdString(GetShaderStageName(
                            resource->m_AttachedShaders[i].second)) +
                            QString(" Shader ") +
                            QString::number(
                                    resource->m_AttachedShaders[i].first));
        }
    }

    if (!resource->mLinkStatus.second) {
        m_Ui.labelLinkStatus->setText(tr("Link status: failed"));
    } else {
        m_Ui.labelLinkStatus->setText(tr("Link status: success"));
    }

    m_Ui.tableWidgetUniforms->setRowCount(static_cast<int>(resource->m_Uniforms.size()));
    for (size_t i = 0; i < resource->m_Uniforms.size(); i++) {
        QTableWidgetItem* item =
                new QTableWidgetItem(resource->m_Uniforms[i].m_name.c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(static_cast<int>(i), kUniformTable_NameIdx, item);

        item = new QTableWidgetItem(
                GetGLEnumName(resource->m_Uniforms[i].m_type).c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(static_cast<int>(i), kUniformTable_TypeIdx, item);


        {
            std::ostringstream locStream; 
            locStream << resource->m_Uniforms[i].m_location;
            item = new QTableWidgetItem(
                locStream.str().c_str());
            item->setFlags(Qt::ItemIsEnabled);
            m_Ui.tableWidgetUniforms->setItem(static_cast<int>(i), kUniformTable_LocationIdx, item);
        }
        

        if (resource->m_Uniforms[i].m_supportedType) {
            std::ostringstream valStream;
            valStream << std::showpoint;
            for (size_t j = 0; j < resource->m_Uniforms[i].m_value.size();
                 j++) {
                if (resource->m_Uniforms[i].m_rowSize > 1 && 
                    j % resource->m_Uniforms[i].m_rowSize == 0)
                    valStream << std::endl;
                else if (j)
                    valStream << ", ";
                resource->m_Uniforms[i].m_value[j].writeToSS(valStream);
            }
            item = new QTableWidgetItem(valStream.str().c_str());
        } else {
            item = new QTableWidgetItem(tr("Not supported by debugger"));
        }
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui.tableWidgetUniforms->setItem(static_cast<int>(i), kUniformTable_ValueIdx, item);
    }
    m_Ui.tableWidgetUniforms->resizeRowsToContents();
}

void DGLProgramViewItem::onRequestFinished(
        const dglnet::message::utils::ReplyBase*) {
    m_Listener->fire();
}

void DGLProgramViewItem::onRequestFailed(
    const std::string& msg) {
    QMessageBox::critical(this, "Cannot link program",
        QString::fromStdString(msg));
}

void DGLProgramViewItem::forceLink() {
    m_RequestManager->request(new dglnet::request::ForceLinkProgram(
                                      m_Name.m_Context, m_Name.m_Name),
                              this);
}

DGLProgramView::DGLProgramView(QWidget* parrent, DglController* controller)
        : DGLTabbedView(parrent, controller) {
    setupNames("Shader Programs", "DGLProgramView");

    // inbound
    CONNASSERT(controller->getViewRouter(),
               SIGNAL(showProgram(opaque_id_t, gl_t)), this,
               SLOT(showProgram(opaque_id_t, gl_t)));
}

void DGLProgramView::showProgram(opaque_id_t ctx, gl_t name) {
    ensureTabDisplayed(ctx, name);
}

DGLTabbedViewItem* DGLProgramView::createTab(
        const dglnet::ContextObjectName& id) {
    return new DGLProgramViewItem(id, m_Controller->getResourceManager(), this);
}

QString DGLProgramView::getTabName(gl_t id, gl_t /*target*/) {
    return QString("Program Shader ") + QString::number(id);
}
