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



DGLResourceListener::DGLResourceListener(uint listenerId, ContextObjectName objectName, DGLResource::ObjectType type, DGLResourceManager* manager):
m_ListenerId(listenerId), m_ObjectName(objectName), m_ObjectType(type), m_Manager(manager), m_RefCount(0) {
    m_Manager->registerListener(this);
}

DGLResourceListener::~DGLResourceListener() {
    m_Manager->unregisterListener(this);
}

DGLResourceManager::DGLResourceManager(DglController* controller):m_MaxListenerId(0), m_Controller(controller) {}

void DGLResourceManager::emitQueries() {
    dglnet::QueryResourceMessage queries;
    for (std::multimap<uint, DGLResourceListener*>::iterator i = m_Listeners.begin(); i != m_Listeners.end(); i++) {
        dglnet::QueryResourceMessage::ResourceQuery query;
        query.m_ListenerId = i->first;
        query.m_ObjectName = i->second->m_ObjectName;
        query.m_Type = i->second->m_ObjectType;
        queries.m_ResourceQueries.push_back(query);
    }
    if (queries.m_ResourceQueries.size())
        m_Controller->sendMessage(&queries);
}


void DGLResourceManager::handleResourceMessage(const dglnet::ResourceMessage& msg) {

    int  idx = 0; 
    bool end = false;

    //this strange method of iteration is to allow modification of m_Listeners

    for (int idx = 0; !end; idx++) {
        std::pair<std::multimap<uint, DGLResourceListener*>::iterator, std::multimap<uint, DGLResourceListener*>::iterator> range = m_Listeners.equal_range(msg.m_ListenerId);
        std::multimap<uint, DGLResourceListener*>::iterator i  = range.first;
        for (int j = 0; j < idx && i != range.second ; j++) {
            i++;
        }
        if (i == range.second) {
            end = true; 
        } else {
            std::string errorMessage;
            if (msg.isOk(errorMessage)) {
                i->second->update(*msg.m_Resource);
            } else {
                i->second->error(errorMessage);
            }
        }
    }
}

DGLResourceListener* DGLResourceManager::createListener(ContextObjectName name, DGLResource::ObjectType type) {
    for (std::multimap<uint, DGLResourceListener*>::iterator i = m_Listeners.begin(); i != m_Listeners.end(); i++) {
        if (i->second->m_ObjectName == name && i->second->m_ObjectType == type) {
            return new DGLResourceListener(i->second->m_ListenerId, name, type, this);
        }
    }
    return new DGLResourceListener(m_MaxListenerId++, name, type, this);
}

void DGLResourceManager::registerListener(DGLResourceListener* listener) {
    m_Listeners.insert(std::pair<uint, DGLResourceListener*>(listener->m_ListenerId, listener));

    dglnet::QueryResourceMessage queries;
    dglnet::QueryResourceMessage::ResourceQuery query;
    query.m_ListenerId = listener->m_ListenerId;
    query.m_ObjectName = listener->m_ObjectName;
    query.m_Type = listener->m_ObjectType;
    queries.m_ResourceQueries.push_back(query);
    m_Controller->sendMessage(&queries);
}

void DGLResourceManager::unregisterListener(DGLResourceListener* listener) {
    std::pair<std::multimap<uint, DGLResourceListener*>::iterator, std::multimap<uint, DGLResourceListener*>::iterator> range = m_Listeners.equal_range(listener->m_ListenerId);
    for (std::multimap<uint, DGLResourceListener*>::iterator i = range.first; i != range.second; i++) {
        if (i->second == listener) {
            m_Listeners.erase(i);
            break;
        }
    }
}

void DGLViewRouter::show(const ContextObjectName& name, DGLResource::ObjectType type) {
    switch (type) {
        case DGLResource::ObjectTypeBuffer:
            emit showBuffer(name.m_Context, name.m_Name);
            break;
        case DGLResource::ObjectTypeFramebuffer:
            emit showFramebuffer(name.m_Context, name.m_Name);
            break;
        case DGLResource::ObjectTypeFBO:
            emit showFBO(name.m_Context, name.m_Name);
            break;
        case DGLResource::ObjectTypeTexture:
            emit showTexture(name.m_Context, name.m_Name);
            break;
        case DGLResource::ObjectTypeShader:
            emit showShader(name.m_Context, name.m_Name, name.m_Target);
            break;
        case DGLResource::ObjectTypeProgram:
            emit showProgram(name.m_Context, name.m_Name);
            break;
    }
}

DglController::DglController():m_Disconnected(true), m_Connected(false), m_ConfiguredAndBkpointsSet(false), m_BreakPointController(this), m_ResourceManager(this) {
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
    dglnet::ContinueBreakMessage message(false);
    m_DglClient->sendMessage(&message);
}

void DglController::debugInterrupt() {
    assert(isConnected());
    dglnet::ContinueBreakMessage message(true);
    m_DglClient->sendMessage(&message);
    newStatus("Interrupting...");
}

void DglController::debugStep() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::ContinueBreakMessage message(dglnet::ContinueBreakMessage::STEP_CALL);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::debugStepDrawCall() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::ContinueBreakMessage message(dglnet::ContinueBreakMessage::STEP_DRAW_CALL);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::debugStepFrame() {
    setBreaked(false);
    setRunning(true);
    assert(isConnected());
    dglnet::ContinueBreakMessage message(dglnet::ContinueBreakMessage::STEP_FRAME);
    m_DglClient->sendMessage(&message);
    newStatus("Running...");
}

void DglController::onSetStatus(std::string str) {
    newStatus(str.c_str());
}

void DglController::queryCallTrace(uint startOffset, uint endOffset) {
    dglnet::QueryCallTraceMessage message(startOffset, endOffset);
    m_DglClient->sendMessage(&message);
}

void DglController::doHandle(const dglnet::HelloMessage & msg) {
    //we are connected now
    m_Connected = true;
    debugeeInfo(msg.m_ProcessName);
    setConnected(true); 
    setDisconnected(false);
}

void DglController::doHandle(const dglnet::BreakedCallMessage & msg) {
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

void DglController::doHandle(const dglnet::CallTraceMessage& msg) {
    gotCallTraceChunkChunk(msg.m_StartOffset, msg.m_Trace);
}

void DglController::doHandle(const dglnet::ResourceMessage& msg) {
    m_ResourceManager.handleResourceMessage(msg);
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

DGLResourceManager* DglController::getResourceManager() {
    return &m_ResourceManager;
}

DGLViewRouter* DglController::getViewRouter() {
    return &m_ViewRouter;
}

void DglController::configure(const DGLConfiguration& config) {
    m_Config = config;
    sendConfig();
}

void DglController::sendConfig() {
    if (isConnected()) {
        dglnet::ConfigurationMessage message(m_Config);
        m_DglClient->sendMessage(&message);
    }
}

const DGLConfiguration& DglController::getConfig() {
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
        dglnet::SetBreakPointsMessage msg(m_Current);
        m_Controller->sendMessage(&msg);
    }    
}