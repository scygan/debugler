#include "dglcontroller.h"
#include "dglgui.h"

DglController::DglController():m_DglClientDead(false),m_BreakPointController(this) {
    m_Timer.setInterval(10);
    CONNASSERT(connect(&m_Timer, SIGNAL(timeout()), this, SLOT(poll())));
}

void DglController::connectServer(const std::string& host, const std::string& port) {
    if (m_DglClient) {
        disconnectServer();
    }
    m_DglClientDead = false;
    m_DglClient = boost::make_shared<dglnet::Client>(this, this);
    m_DglClient->connectServer(host, port);
    m_Timer.start();
    connected();
}

void DglController::onSocket() {
    //m_Timer.stop();
    m_NotifierRead = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Read); 
    m_NotifierWrite = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Write); 
    CONNASSERT(connect(&*m_NotifierRead, SIGNAL(activated(int)), this, SLOT(poll())));
    CONNASSERT(connect(&*m_NotifierWrite, SIGNAL(activated(int)), this, SLOT(poll())));
}

void DglController::disconnectServer() {
    if (m_DglClient) {
        m_DglClient->abort();
        m_DglClient.reset();
        disconnected();
    }
    m_NotifierRead.reset();
    m_NotifierWrite.reset();
}

void DglController::poll() {
    if (m_DglClient) {
        m_DglClient->poll();
        if (m_DglClientDead)  {
            disconnectServer();
            error(tr("Connection error"), m_DglClientDeadInfo.c_str());
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

void DglController::doHandle(const dglnet::TextureMessage& msg) {
    gotTexture(msg.m_TextureName, msg);
}

void DglController::doHandle(const dglnet::BufferMessage& msg) {
    gotBuffer(msg.m_BufferName, msg);
}

void DglController::doHandle(const dglnet::FramebufferMessage& msg) {
    gotFramebuffer(msg.m_BufferEnum, msg);
}

void DglController::doHandle(const dglnet::FBOMessage& msg) {
    gotFBO(msg.m_Name, msg);
}

void DglController::doHandleDisconnect(const std::string& msg) {
    m_DglClientDeadInfo = msg;
    m_DglClientDead = true; 
}

void DglController::doShowTexture(uint name) {
    assert(m_DglClient);
    dglnet::QueryTextureMessage message(name);
    m_DglClient->sendMessage(&message);
    showTexture(name);
}

void DglController::doShowBuffer(uint name) {
    assert(m_DglClient);
    dglnet::QueryBufferMessage message(name);
    m_DglClient->sendMessage(&message);

    showBuffer(name);
}

void DglController::doShowFramebuffer(GLenum type) {
    assert(m_DglClient);
    dglnet::QueryFramebufferMessage message(type);
    m_DglClient->sendMessage(&message);

    showFramebuffer(type);
}

void DglController::doShowFBO(uint name) {
    assert(m_DglClient);
    dglnet::QueryFBOMessage message(name);
    m_DglClient->sendMessage(&message);

    showFBO(name);
} 

void DglController::sendMessage(dglnet::Message* msg) {
    assert(m_DglClient);
    m_DglClient->sendMessage(msg);
}

DGLBreakPointController* DglController::getBreakPoints() {
    return &m_BreakPointController;
}

DGLBreakPointController::DGLBreakPointController(DglController* controller):m_Controller(controller) {}

std::set<Entrypoint> DGLBreakPointController::getCurrent() {
    return m_Current;
}

void DGLBreakPointController::setCurrent(const std::set<Entrypoint>& newCurrent) {
    if (m_Current != newCurrent) {
        m_Current = newCurrent;
        dglnet::SetBreakPointsMessage msg(m_Current);
        m_Controller->sendMessage(&msg);
    }
}