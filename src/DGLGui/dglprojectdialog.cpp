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

DGLProjectDialog::DGLProjectDialog() {
    m_ui.setupUi(this);

    addProjectFactory(std::make_shared<DGLTCPConnectionProjectFactory>());
}

DGLProjectDialog::~DGLProjectDialog() {}


void DGLProjectDialog::projectTypeSelected(int row) {
    if (row >= 0 && row < static_cast<int>(m_Factories.size())) {
        m_ui.groupBoxProjParameters->layout()->addWidget(m_Factories[row]->getGUI());
    }
}

void DGLProjectDialog::addProjectFactory(std::shared_ptr<DGLProjectFactory> factory) {
    m_Factories.push_back(factory);
    m_ui.listWidgetProjectType->addItem(factory->getName());

    projectTypeSelected(m_ui.listWidgetProjectType->currentRow());
}


std::shared_ptr<DGLProject> DGLProjectDialog::getProject() {
    return std::shared_ptr<DGLProject>();
}