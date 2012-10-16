
#include "dglconnectdialog.h"

DGLConnectDialog::DGLConnectDialog() {
    m_ui.setupUi(this);
    m_ui.lineEdit_TcpPort->setValidator(&m_portValidator);
}

DGLConnectDialog::~DGLConnectDialog() {}

std::string DGLConnectDialog::getAddress() {
    return m_ui.lineEdit_IpAddress->text().toStdString();

}

std::string DGLConnectDialog::getPort()
{
    return m_ui.lineEdit_TcpPort->text().toStdString();
}
