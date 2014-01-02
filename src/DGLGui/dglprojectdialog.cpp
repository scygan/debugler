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

#include "dglprojectdialog.h"

#include "dglproject_attach.h"
#include "dglproject_runapp.h"
#include "dglproject_android.h"

#include <QMessageBox>

DGLProjectDialog::DGLProjectDialog():m_CurrentFactory(nullptr) {
    m_ui.setupUi(this);

    addProjectFactory(std::make_shared<DGLRunAppProjectFactory>());
    addProjectFactory(std::make_shared<DGLAttachProjectFactory>());
    addProjectFactory(std::make_shared<DGLAndroidProjectFactory>());
}

DGLProjectDialog::~DGLProjectDialog() {}


void DGLProjectDialog::projectTypeSelected(int row) {
    if (row >= 0 && row < static_cast<int>(m_Factories.size())) {
        m_CurrentFactory = m_Factories[row].get();
        m_ui.stackedWidget->setCurrentIndex(row);
    }
}

void DGLProjectDialog::tryAccept() {
    if (!m_CurrentFactory) {
        QMessageBox::critical(this, tr("Project type not selected"),
            tr("Please select project type from list."));
    } else {
        QString msg;
        if (!m_CurrentFactory->valid(msg)) {
            QMessageBox::critical(this, tr("Project properties invalid"),
                msg);
        } else {
            accept();
        }
    }
}


void DGLProjectDialog::addProjectFactory(std::shared_ptr<DGLProjectFactory> factory) {
    m_Factories.push_back(factory);
    m_ui.listWidgetProjectType->addItem(factory->getName());
    m_ui.stackedWidget->addWidget(factory->getGUI());

    projectTypeSelected(m_ui.listWidgetProjectType->currentRow());
}


std::shared_ptr<DGLProject> DGLProjectDialog::getProject() {
    if (!m_CurrentFactory) {
        throw std::runtime_error("No project factory selected");
    } else {
        return m_CurrentFactory->createProject();
    }
}

void DGLProjectDialog::loadPropertiesFromProject(const DGLProject* project) {
    for (size_t i = 0; i < m_Factories.size(); i++) {
        if (m_Factories[i]->loadPropertiedFromProject(project)) {
            return;
        }
    }
    throw std::runtime_error("No factory cared about this project type.");
}