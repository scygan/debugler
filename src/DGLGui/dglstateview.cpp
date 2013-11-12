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

#include "dglstateview.h"

#include <set>
#include <climits>
#include <iomanip>
#include <sstream>

DGLStateView::DGLStateView(QWidget* parrent, DglController* controller)
        : QDockWidget(tr("OpenGL State"), parrent),
          m_Listener(NULL),
          m_Controller(controller),
          m_Ui(NULL) {
    setObjectName("DGLStateView");

    setConnected(false);

    // inbound
    CONNASSERT(controller, SIGNAL(setConnected(bool)), this,
               SLOT(setConnected(bool)));
}

void DGLStateView::update(const dglnet::DGLResource& res) {
    const dglnet::resource::DGLResourceState* resource =
        dynamic_cast<const dglnet::resource::DGLResourceState*>(&res);

    bool initializeRows = !(m_Ui->tableWidget->rowCount());

    if (initializeRows) {
        m_Ui->tableWidget->setRowCount(resource->m_Items.size());

        for (size_t i = 0; i < resource->m_Items.size(); i++) {
            QTableWidgetItem* item =
                new QTableWidgetItem(resource->m_Items[i].m_Name.c_str());
            item->setFlags(Qt::ItemIsEnabled);
            m_Ui->tableWidget->setItem(i, 0, item);
        }
    }

    for (size_t i = 0; i < resource->m_Items.size(); i++) {
        std::ostringstream valStream;
        valStream << std::showpoint;
        for (size_t j = 0; j < resource->m_Items[i].m_Values.size(); j++) {
            if (j) valStream << ", ";
            resource->m_Items[i].m_Values[j].writeToSS(valStream);
            valStream << " ";
        }
        QTableWidgetItem* item = new QTableWidgetItem(valStream.str().c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui->tableWidget->setItem(i, 1, item);
    }
}

void DGLStateView::error(const std::string& /*message*/) {
    m_Ui->tableWidget->setRowCount(0);
}

void DGLStateView::setConnected(bool connected) {
    if (!connected) {
        if (m_Ui) {
            delete m_Ui->frame;
            delete m_Ui;
            m_Ui = NULL;
        }
    } else {
        m_Ui = new Ui::DGLStateView();
        m_Ui->setupUi(this);
        setWidget(m_Ui->frame);
        setLayout(m_Ui->verticalLayout);
        m_Ui->tableWidget->setRowCount(1);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        m_Ui->tableWidget->horizontalHeader()->setResizeMode(
            QHeaderView::Stretch);
#else
        m_Ui->tableWidget->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
#endif
        m_Ui->tableWidget->setHorizontalHeaderItem(
            0, new QTableWidgetItem("Parameter"));
        m_Ui->tableWidget->setHorizontalHeaderItem(
            1, new QTableWidgetItem("Value"));

        m_Listener = m_Controller->getResourceManager()->createListener(
            dglnet::ContextObjectName(),
            dglnet::DGLResource::ObjectType::State);
        m_Listener->setParent(m_Ui->frame);

        CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
                   SLOT(update(const dglnet::DGLResource&)));
        CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
                   SLOT(error(const std::string&)));
    }
}
