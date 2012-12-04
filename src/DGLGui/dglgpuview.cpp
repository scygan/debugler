#include "dglgpuview.h"
#include "dglgui.h"

#include <set>
#include <climits>

DGLGPUView::DGLGPUView(QWidget* parrent, DglController* controller):QDockWidget(tr("State Tree"), parrent), m_Listener(NULL), m_Controller(controller) {
    setObjectName("DGLGPUView");

    m_Ui.setupUi(this);
    setWidget(m_Ui.groupBox);

    setConnected(false);
   
    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
}

void DGLGPUView::update(const DGLResource& res) {
    const DGLResourceGPU* resource = dynamic_cast<const DGLResourceGPU*>(&res);
    m_Ui.lineEdit_Renderer->setText(QString::fromStdString(resource->m_Renderer));
    m_Ui.lineEdit_Vendor->setText(QString::fromStdString(resource->m_Vendor));
    m_Ui.lineEdit_Version->setText(QString::fromStdString(resource->m_Version));
}

void DGLGPUView::error(const std::string& message) {
    m_Ui.lineEdit_Renderer->setText(QString::fromStdString(message));
    m_Ui.lineEdit_Vendor->clear();
    m_Ui.lineEdit_Version->clear();
}

void DGLGPUView::setConnected(bool connected) {
    if (!connected) {
        m_Ui.lineEdit_Renderer->clear();
        m_Ui.lineEdit_Vendor->clear();
        m_Ui.lineEdit_Version->clear();
        if (m_Listener) {
            m_Listener->deleteLater();
            m_Listener = NULL;
        }
    } else {

        m_Listener = m_Controller->getResourceManager()->createListener(0, DGLResource::ObjectTypeGPU);
        m_Listener->setParent(this);

        CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
        CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
    }
}

