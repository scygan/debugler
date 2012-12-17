#include "DGLStateView.h"
#include "dglgui.h"

#include <set>
#include <climits>

DGLStateView::DGLStateView(QWidget* parrent, DglController* controller):QDockWidget(tr("OpenGL State"), parrent), m_Listener(NULL), m_Controller(controller), m_Ui(NULL) {
    setObjectName("DGLStateView");

    setConnected(false);
   
    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
}

void DGLStateView::update(const DGLResource& res) {
    const DGLResourceState* resource = dynamic_cast<const DGLResourceState*>(&res);

    m_Ui->tableWidget->clear();
    m_Ui->tableWidget->setRowCount(resource->m_Items.size());
    m_Ui->tableWidget->setColumnCount(2);
    for (size_t i = 0; i < resource->m_Items.size(); i++) {
        m_Ui->tableWidget->setItem(i,0,new QTableWidgetItem(resource->m_Items[i].m_Name.c_str()));
        std::stringstream valStream;
        for (int j = 0; j < resource->m_Items[i].m_Values.size(); j++) {
            valStream << resource->m_Items[i].m_Values[j] << " ";
        }
        m_Ui->tableWidget->setItem(i,1,new QTableWidgetItem(valStream.str().c_str()));
    }
    //m_Ui->label_Renderer->setText(QString::fromStdString(resource->m_Renderer));
    //m_Ui->label_Vendor->setText(QString::fromStdString(resource->m_Vendor));
    //m_Ui->label_Version->setText(QString::fromStdString(resource->m_Version));

    
}

void DGLStateView::error(const std::string& message) {
    //m_Ui->label_Renderer->setText(QString::fromStdString(message));
    //m_Ui->label_Vendor->clear();
    //m_Ui->label_Version->clear();
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
        
        m_Listener = m_Controller->getResourceManager()->createListener(0, DGLResource::ObjectTypeState);
        m_Listener->setParent(m_Ui->groupBox);

        CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
        CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
    }
}

