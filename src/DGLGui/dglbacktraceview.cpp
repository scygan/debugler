/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include "dglbacktraceview.h"

#include <DGLNet/protocol/resource.h>

#include <set>
#include <climits>

DGLBackTraceView::DGLBackTraceView(QWidget* parrent, DglController* controller)
        : QDockWidget(tr("CPU back trace"), parrent),
          m_Listener(NULL),
          m_Controller(controller),
          m_Ui(NULL) {
    setObjectName("DGLBackTraceView");

    setConnected(false);

    // inbound
    CONNASSERT(controller, SIGNAL(setConnected(bool)), this,
               SLOT(setConnected(bool)));
}

void DGLBackTraceView::update(const dglnet::DGLResource& res) {
    const dglnet::resource::DGLResourceBacktrace* resource =
            dynamic_cast<const dglnet::resource::DGLResourceBacktrace*>(&res);

    m_Ui->listWidget->clear();

    for (size_t i = 0; i < resource->m_trace.size(); i++) {
        m_Ui->listWidget->addItem(QString::fromStdString(resource->m_trace[i]));
    }
}

void DGLBackTraceView::error(const std::string& message) {

    m_Ui->listWidget->clear();

    m_Ui->listWidget->addItem(QString::fromStdString(message));
}

void DGLBackTraceView::setConnected(bool connected) {
    if (!connected) {
        if (m_Ui) {
            delete m_Ui->frame;
            delete m_Ui;
            m_Ui = NULL;
        }
    } else {
        m_Ui = new Ui::DGLBacktraceView();
        m_Ui->setupUi(this);
        setWidget(m_Ui->frame);
        setLayout(m_Ui->verticalLayout);

        m_Listener = m_Controller->getResourceManager()->createListener(
                dglnet::ContextObjectName(),
                dglnet::message::ObjectType::BackTrace);
        m_Listener->setParent(m_Ui->frame);

        CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
                   SLOT(update(const dglnet::DGLResource&)));
        CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
                   SLOT(error(const std::string&)));

        m_Listener->setEnabled(isVisible());
        CONNASSERT(this, SIGNAL(visibilityChanged(bool)), m_Listener,
            SLOT(setEnabled(bool)));
    }
}
