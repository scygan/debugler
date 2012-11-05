#include "dglcontroller.h"

DglController::DglController():m_DglClientDead(false) {
    m_Timer.setInterval(10);
    assert(connect(&m_Timer, SIGNAL(timeout()), this, SLOT(poll())));
    m_Timer.start();
}

void DglController::connectServer(const std::string& host, const std::string& port) {
    if (m_DglClient) {
        disconnectServer();
    }
    m_DglClient = boost::make_shared<dglnet::Client>(this, this);
    m_DglClient->connectServer(host, port);

    //m_Timer.stop();

    connected();
    //m_NotifierRead = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Read); 
    //m_NotifierWrite = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Write); 
    //assert(connect(&*m_NotifierRead, SIGNAL(activated(int)), this, SLOT(poll())));
    //assert(connect(&*m_NotifierWrite, SIGNAL(activated(int)), this, SLOT(poll())));
}

void DglController::disconnectServer() {
    if (m_DglClient) {
        m_DglClient->disconnect();
        m_DglClient.reset();
        disconnected();
    }
}

void DglController::poll() {
    if (m_DglClient) {
        m_DglClient->poll();
        if (m_DglClientDead)  {
            m_DglClientDead = true;
            disconnectServer();
        }
    }
}

void DglController::debugContinue() {
    assert(m_DglClient);
    dglnet::ContinueBreakMessage message(false);
    m_DglClient->sendMessage(&message);
}

void DglController::debugInterrupt() {
    assert(m_DglClient);
    dglnet::ContinueBreakMessage message(true);
    m_DglClient->sendMessage(&message);
}

void DglController::debugStep() {
    assert(m_DglClient);
    dglnet::ContinueBreakMessage message(false, true);
    m_DglClient->sendMessage(&message);
}

void DglController::onSetStatus(std::string str) {
    newStatus(str.c_str());
}

void DglController::queryCallTrace(uint startOffset, uint endOffset) {
    dglnet::QueryCallTraceMessage message(startOffset, endOffset);
    m_DglClient->sendMessage(&message);
}

void DglController::doHandle(const dglnet::BreakedCallMessage & msg) {
    breaked(msg.m_entryp, msg.m_TraceSize);
    breakedWithStateReports(msg.m_CurrentCtx, msg.m_CtxReports);
}

void DglController::doHandle(const dglnet::CallTraceMessage& msg) {
    gotCallTraceChunkChunk(msg.m_StartOffset, msg.m_Trace);
}

void DglController::doHandleDisconnect(const std::string& msg) {
    error(tr("Connection error"), msg.c_str());
    m_DglClientDead = true; 
}