#include "dglgpuview.h"
#include "dglgui.h"

#include <set>
#include <climits>

DGLGPUView::DGLGPUView(QWidget* parrent, DglController* controller):QDockWidget(tr("OpenGL and GPU"), parrent), m_Listener(NULL), m_Controller(controller), m_Ui(NULL) {
    setObjectName("DGLGPUView");

    setConnected(false);
   
    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
}

void DGLGPUView::update(const DGLResource& res) {
    const DGLResourceGPU* resource = dynamic_cast<const DGLResourceGPU*>(&res);
    m_Ui->label_Renderer->setText(QString::fromStdString(resource->m_Renderer));
    m_Ui->label_Vendor->setText(QString::fromStdString(resource->m_Vendor));
    m_Ui->label_Version->setText(QString::fromStdString(resource->m_Version));

    if (!resource->m_hasNVXGPUMemoryInfo) {
        m_Ui->groupBox_NVMem->hide();
    } else {
        m_Ui->groupBox_NVMem->show();
        m_Ui->label_Dedicated->setText(QString::number(resource->m_nvidiaMemory.memInfoTotalAvailMem / 1000) + QString(" MB"));
        m_Ui->label_Eviction->setText(QString::number(resource->m_nvidiaMemory.memInfoEvictedMem / 1000) + QString(" MB,  (") + 
            QString::number(resource->m_nvidiaMemory.memInfoEvictionCount) + QString(" evictions)"));
        m_Ui->progressBar_VidMem->setMaximum(resource->m_nvidiaMemory.memInfoDedidactedVidMem / 1000);
        m_Ui->progressBar_VidMem->setValue(resource->m_nvidiaMemory.memInfoCurrentAvailVidMem / 1000);
    }
}

void DGLGPUView::error(const std::string& message) {
    m_Ui->label_Renderer->setText(QString::fromStdString(message));
    m_Ui->label_Vendor->clear();
    m_Ui->label_Version->clear();
    m_Ui->groupBox_NVMem->hide();
}

void DGLGPUView::setConnected(bool connected) {
    if (!connected) {
        if (m_Ui) {
            delete m_Ui->frame;
            delete m_Ui;
            m_Ui = NULL;
        }
    } else {
        m_Ui = new Ui::DGLGPUView();
        m_Ui->setupUi(this);
        m_Ui->groupBox_NVMem->hide();
        setWidget(m_Ui->frame);
        setLayout(m_Ui->verticalLayout);
        
        m_Listener = m_Controller->getResourceManager()->createListener(0, DGLResource::ObjectTypeGPU);
        m_Listener->setParent(m_Ui->groupBox);

        CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
        CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
    }
}
