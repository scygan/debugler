#include "dglstateview.h"
#include "dglgui.h"

#include <set>
#include <climits>
#include <iomanip>

DGLStateView::DGLStateView(QWidget* parrent, DglController* controller):QDockWidget(tr("OpenGL State"), parrent), m_Listener(NULL), m_Controller(controller), m_Ui(NULL) {
    setObjectName("DGLStateView");
    
    setConnected(false);
   
    //inbound
    CONNASSERT(connect(controller, SIGNAL(setConnected(bool)), this, SLOT(setConnected(bool))));
}

void DGLStateView::update(const DGLResource& res) {
    const DGLResourceState* resource = dynamic_cast<const DGLResourceState*>(&res);

    bool initializeRows = !(m_Ui->tableWidget->rowCount());
    
    if (initializeRows) {
        m_Ui->tableWidget->setRowCount(resource->m_Items.size());

        for (size_t i = 0; i < resource->m_Items.size(); i++) { 
            QTableWidgetItem * item = new QTableWidgetItem(resource->m_Items[i].m_Name.c_str());
            item->setFlags(Qt::ItemIsEnabled);
            m_Ui->tableWidget->setItem(i, 0, item);
        }
    }    

    for (size_t i = 0; i < resource->m_Items.size(); i++) {
        std::ostringstream valStream;
        valStream << std::showpoint;
        for (int j = 0; j < resource->m_Items[i].m_Values.size(); j++) {
            if (j)
                valStream << ", ";
            resource->m_Items[i].m_Values[j].writeToSS(valStream);
            valStream << " ";
        }
        QTableWidgetItem * item = new QTableWidgetItem(valStream.str().c_str());
        item->setFlags(Qt::ItemIsEnabled);
        m_Ui->tableWidget->setItem(i, 1, item);
    }

}

void DGLStateView::error(const std::string& message) {
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
        m_Ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Parameter"));
        m_Ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
        
        m_Listener = m_Controller->getResourceManager()->createListener(ContextObjectName(), DGLResource::ObjectTypeState);
        m_Listener->setParent(m_Ui->frame);

        CONNASSERT(connect(m_Listener,SIGNAL(update(const DGLResource&)),this,SLOT(update(const DGLResource&))));
        CONNASSERT(connect(m_Listener,SIGNAL(error(const std::string&)),this,SLOT(error(const std::string&))));
    }
}

