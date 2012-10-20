#include "dglcontroller.h"

DglController::DglController() {
    m_Timer.setInterval(10);
    assert(connect(&m_Timer, SIGNAL(timeout()), this, SLOT(poll())));
    m_Timer.start();
}

void DglController::connectClient(const std::string& host, const std::string& port) {
    if (m_DglClient) {
        disconnected();
    }
    m_DglClient = boost::make_shared<dglnet::Client>(host, port, this);

    

    //m_NotifierRead = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Read); 
    //m_NotifierWrite = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Read); 




}

void DglController::poll() {
    if (m_DglClient)
        m_DglClient->poll();
}

void DglController::onSetStatus(std::string str) {
    newStatus(str.c_str());
}


void DglController::onInternalError(std::string str) {
    error(tr("Connection error"), str.c_str());
}