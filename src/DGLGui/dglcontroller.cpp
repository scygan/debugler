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


#include "dglcontroller.h"
#include "dglgui.h"

DGLRequestHandler::DGLRequestHandler(DGLRequestManager* manager):m_Manager(manager) {}

DGLRequestHandler::~DGLRequestHandler() {
    m_Manager->unregisterHandler(this);
}

DGLRequestManager::DGLRequestManager(DglController* controller):m_Controller(controller) {}

void DGLRequestManager::request(dglnet::DGLRequest* request, DGLRequestHandler* handler) {

    dglnet::message::Request requestMessage(request);
    m_CurrentHandlers[requestMessage.getId()] = handler;
    m_Controller->sendMessage(&requestMessage);
}

void DGLRequestManager::handle(const dglnet::message::RequestReply& msg) {
    std::map<int, DGLRequestHandler*>::iterator i = m_CurrentHandlers.find(msg.getId());
    if (i != m_CurrentHandlers.end()) {
        i->second->onRequestFinished(&msg);
        m_CurrentHandlers.erase(i);
    }
}

void DGLRequestManager::unregisterHandler(DGLRequestHandler* handler) {
    for (std::map<int, DGLRequestHandler*>::iterator i = m_CurrentHandlers.begin(); i != m_CurrentHandlers.end(); i++) {
        if (i->second == handler) {
            m_CurrentHandlers.erase(i++);
        }
    }
}


DGLResourceListener::DGLResourceListener(dglnet::ContextObjectName objectName, dglnet::DGLResource::ObjectType type, DGLResourceManager* manager):DGLRequestHandler(manager->getRequestManager()), m_ObjectType(type), m_ObjectName(objectName), m_Manager(manager) {}

DGLResourceListener::~DGLResourceListener() {
    m_Manager->unregisterListener(this);
}

void DGLResourceListener::onRequestFinished(const dglnet::message::RequestReply* msg) {
    std::string errorMessage;
    if (msg->isOk(errorMessage)) {
        update(*dynamic_cast<dglnet::DGLResource*>(msg->m_Reply.get()));
    } else {
        error(errorMessage);
    }
}

DGLResourceManager::DGLResourceManager(DGLRequestManager* manager): m_RequestManager(manager) {}

void DGLResourceManager::emitQueries() {
    for (std::list<DGLResourceListener*>::iterator i = m_Listeners.begin(); i != m_Listeners.end(); i++) {
        m_RequestManager->request(new dglnet::request::QueryResource((*i)->m_ObjectType, (*i)->m_ObjectName), *i);
    }
}

DGLResourceListener* DGLResourceManager::createListener(dglnet::ContextObjectName name, dglnet::DGLResource::ObjectType type) {
    DGLResourceListener* listener = new DGLResourceListener(name, type, this);
    
    m_Listeners.insert(m_Listeners.end(), listener);
    m_RequestManager->request(new dglnet::request::QueryResource(listener->m_ObjectType, listener->m_ObjectName), listener);

    return listener;
}

DGLRequestManager* DGLResourceManager::getRequestManager() {
    return m_RequestManager;    
}

void DGLResourceManager::unregisterListener(DGLResourceListener* listener) {
    for (std::list<DGLResourceListener*>::iterator i = m_Listeners.begin(); i != m_Listeners.end(); i++) {
        if (*i == listener) {
            m_Listeners.erase(i);
            break;
        }
    }
}

void DGLViewRouter::show(const dglnet::ContextObjectName& name, dglnet::DGLResource::ObjectType type) {
    switch (type) {
        case dglnet::DGLResource::ObjectTypeBuffer:
            emit showBuffer(name.m_Context, name.m_Name);
            break;
        case dglnet::DGLResource::ObjectTypeFramebuffer:
            emit showFramebuffer(name.m_Context, name.m_Name);
            break;
        case dglnet::DGLResource::ObjectTypeFBO:
            emit showFBO(name.m_Context, name.m_Name);
            break;
        case dglnet::DGLResource::ObjectTypeTexture:
            emit showTexture(name.m_Context, name.m_Name);
            break;
        case dglnet::DGLResource::ObjectTypeShader:
            emit showShader(name.m_Context, name.m_Name, name.m_Target);
            break;
        case dglnet::DGLResource::ObjectTypeProgram:
            emit showProgram(name.m_Context, name.m_Name);
            break;
        default:
        	assert(0);
        	break;
    }
}

DglController::DglController():m_Disconnected(true), m_Connected(false), m_ConfiguredAndBkpointsSet(false), m_BreakPointController(this), m_RequestManager(this), m_ResourceManager(getRequestManager()) {
    m_Timer.setInterval(10);
    CONNASSERT(connect(&m_Timer, SIGNAL(timeout()), this, SLOT(poll())));
}

void DglController::connectServer(const std::string& host, const std::string& port) {
    if (m_DglClient) {
        disconnectServer();
    }

    //we are not disconnected, but not yet connected - so we do not set m_Connected
    m_Disconnected = false; 

    m_DglClient = boost::make_shared<dglnet::Client>(this, this);
    m_DglClient->connectServer(host, port);
    m_Timer.start();
}

void DglController::onSocket() {
    
    //Hey! If you are trying to disable the timer-based polling here, please increment the following counter.
    //You would not succeed... on windows some socketnotifies activate()-s are missed, so we got stuck and 
    //wait forever for read data. 

    // Timer disable try count: 2
    // m_Timer.stop();

    m_NotifierRead = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Read); 
    m_NotifierWrite = boost::make_shared<QSocketNotifier>(m_DglClient->getSocketFD(), QSocketNotifier::Write); 
    m_NotifierWrite->setEnabled(false);
    CONNASSERT(connect(&*m_NotifierRead, SIGNAL(activated(int)), this, SLOT(poll())));
    CONNASSERT(connect(&*m_NotifierWrite, SIGNAL(activated(int)), this, SLOT(poll())));
}

void DglController::onSocketStartSend() {
    m_NotifierWrite->setEnabled(true);
}

void DglController::onSocketStopSend() {
    m_NotifierWrite->setEnabled(false);
}

void DglController::disconnectServer() {
    if (m_DglClient) {
        m_DglClient->abort();
        m_DglClient.reset();
        m_ConfiguredAndBkpointsSet = false;
        setConnected(false);
        setDisconnected(true);
        debugeeInfo("");
    }
    m_Connected = false;
    m_NotifierRead.reset();
    m_NotifierWrite.reset();
    newStatus("Disconnected.");
}

bool DglController::isConnected() {
    return m_Connected;
}

void DglController::poll() {
    if (m_DglClient) {
        m_DglClient->poll();

        if (m_Disconnected)  {
            //one of asio handlers requested disconnection
            disconnectServer();
            error(tr("Connection error"), m_DglClientDeadInfo.c_str());
        }
    }
}

void DglController::debugContinue() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::message::ContinueBreak message(false);
    m_DglClient->sendMessage(&message);
}

void DglController::debugInterrupt() {
    assert(isConnected());
    dglnet::message::ContinueBreak message(true);
    m_DglClient->sendMessage(&message);
    newStatus("Interrupting...");
}

void DglController::debugStep() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::message::ContinueBreak message(dglnet::message::ContinueBreak::STEP_CALL);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::debugStepDrawCall() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::message::ContinueBreak message(dglnet::message::ContinueBreak::STEP_DRAW_CALL);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::debugStepFrame() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::message::ContinueBreak message(dglnet::message::ContinueBreak::STEP_FRAME);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::onSetStatus(std::string str) {
    newStatus(str.c_str());
}

void DglController::queryCallTrace(uint startOffset, uint endOffset) {
    dglnet::message::QueryCallTrace message(startOffset, endOffset);
    m_DglClient->sendMessage(&message);
}

void DglController::doHandle(const dglnet::message::Hello & msg) {
    //we are connected now
    m_Connected = true;
    debugeeInfo(msg.m_ProcessName);
    setConnected(true); 
    setDisconnected(false);
}

void DglController::doHandle(const dglnet::message::BreakedCall & msg) {
    if (!m_ConfiguredAndBkpointsSet) {
        //this is the first time debugee was stopped, before any execution
        //we must upload some configuration to it
        m_ConfiguredAndBkpointsSet = true;

        sendConfig();
        getBreakPoints()->sendCurrent();
    }

    setBreaked(true);
    setRunning(false);    

    breaked(msg.m_entryp, msg.m_TraceSize);
    breakedWithStateReports(msg.m_CurrentCtx, msg.m_CtxReports);

    m_ResourceManager.emitQueries();

    newStatus("Breaked execution.");
}

void DglController::doHandle(const dglnet::message::CallTrace& msg) {
    gotCallTraceChunkChunk(msg.m_StartOffset, msg.m_Trace);
}

void DglController::doHandle(const dglnet::message::RequestReply& msg) {
    getRequestManager()->handle(msg);
}

void DglController::doHandleDisconnect(const std::string& msg) {
    m_DglClientDeadInfo = msg;
    m_Disconnected = true; 
    m_Connected = !m_Disconnected;
}

void DglController::sendMessage(dglnet::Message* msg) {
    assert(isConnected());
    m_DglClient->sendMessage(msg);
}

DGLBreakPointController* DglController::getBreakPoints() {
    return &m_BreakPointController;
}

DGLRequestManager* DglController::getRequestManager() {
    return &m_RequestManager;
}

DGLResourceManager* DglController::getResourceManager() {
    return &m_ResourceManager;
}

DGLViewRouter* DglController::getViewRouter() {
    return &m_ViewRouter;
}

void DglController::sendConfig(const DGLConfiguration* config) {
    if (config) {
        m_Config = *config;
    }
    if (isConnected()) {
        dglnet::message::Configuration message(m_Config);
        m_DglClient->sendMessage(&message);
    }
}

DGLConfiguration& DglController::getConfig() {
    return m_Config;
}

DGLBreakPointController::DGLBreakPointController(DglController* controller):m_Controller(controller) {}

std::set<Entrypoint> DGLBreakPointController::getCurrent() {
    return m_Current;
}

void DGLBreakPointController::setCurrent(const std::set<Entrypoint>& newCurrent) {
    if (m_Current != newCurrent) {
        m_Current = newCurrent;
        sendCurrent();
    }
}

void DGLBreakPointController::sendCurrent() {
    if (m_Controller->isConnected()) {
        dglnet::message::SetBreakPoints msg(m_Current);
        m_Controller->sendMessage(&msg);
    }    
}
