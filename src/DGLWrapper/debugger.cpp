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


#include <boost/make_shared.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

#include "debugger.h"
#include "native-surface.h"
#include "tls.h"

boost::shared_ptr<DGLDebugController> _g_Controller;

DGLDebugController* getController() {
    if (!_g_Controller.get()) {
        _g_Controller = boost::make_shared<DGLDebugController>();
    }
    return _g_Controller.get();
};

DGLConfiguration g_Config;

BreakState::BreakState():m_break(true),m_StepModeEnabled(false) {}

bool BreakState::mayBreakAt(const Entrypoint& e) {
    if (m_StepModeEnabled) {
        switch (m_StepMode) {
            case dglnet::ContinueBreakMessage::STEP_CALL:  
                m_break = true;
                break;
            case dglnet::ContinueBreakMessage::STEP_DRAW_CALL:
                if (IsDrawCall(e)) m_break = true;
                break;
            case dglnet::ContinueBreakMessage::STEP_FRAME:  
                if (IsFrameDelimiter(e)) m_break = true;
                break;
        }
    }

    if (m_BreakPoints.find(e) != m_BreakPoints.end()) {
        m_break = true;
    }
    return isBreaked();
}

void BreakState::setBreakAtGLError(GLenum glError) {
    if (glError != GL_NO_ERROR && g_Config.m_BreakOnGLError) {
        m_break = true;
    }
}

void BreakState::setBreakAtDebugOutput()  {
    if (g_Config.m_BreakOnDebugOutput)
        m_break = true;
}

void BreakState::setBreakAtCompilerError()  {
    if (g_Config.m_BreakOnCompilerError)
        m_break = true;
}

bool BreakState::isBreaked() {
    return m_break;
}

void BreakState::handle(const dglnet::ContinueBreakMessage& msg) {
    m_break = msg.isBreaked();
    if (!m_break) {
        if (m_StepModeEnabled = msg.getStep().first) {
            m_StepMode = msg.getStep().second;
        }
    }
}

void BreakState::handle(const dglnet::SetBreakPointsMessage& msg) {
    m_BreakPoints = msg.get();
}

CallHistory::CallHistory() {
    m_cb = boost::circular_buffer<CalledEntryPoint>(CALL_HISTORY_LEN);
};

void CallHistory::add(const CalledEntryPoint& entryp) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_cb.push_back(entryp);
}

void CallHistory::query(const dglnet::QueryCallTraceMessage& query, dglnet::CallTraceMessage& reply) {
    boost::lock_guard<boost::mutex> lock(m_mutex);

    size_t startOffset = reply.m_StartOffset = query.m_StartOffset;
    if (startOffset >= m_cb.size())
        return; //queried non existent elements

    //trim query to sane range:
    size_t endOffset = std::min(static_cast<size_t>(query.m_EndOffset),m_cb.size());

    boost::circular_buffer<CalledEntryPoint>::iterator begin, end;
    end = m_cb.end() - startOffset;
    begin = m_cb.end() - endOffset;
    std::back_insert_iterator<std::vector<CalledEntryPoint> > replyHistory(reply.m_Trace);
    std::copy(begin, end, replyHistory);
}

size_t CallHistory::size() {
    return m_cb.size();
}

void CallHistory::setRetVal(const RetValue& ret) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_cb.back().setRetVal(ret);
}

void CallHistory::setError( GLenum error ) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_cb.back().setError(error);
}

void CallHistory::setDebugOutput(const std::string& message) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_cb.back().setDebugOutput(message);
}

DGLDebugController::DGLDebugController():m_Disconnected(false) {}

DGLDebugController::~DGLDebugController() {
    if (m_Server) {
        m_Server->abort();
        m_Server.reset();
    }
}

void DGLDebugController::doHandleDisconnect(const std::string&) {
    //we have got disconnected from the client. Mark this in controller state and return, so io_service can be freed later
    m_Disconnected = true;
}

dglnet::Server& DGLDebugController::getServer() {
    if (!m_Server) {

        std::string port = "5555";

        {
            //port may be overrided by environment

            std::string tmp = Os::getEnv("dgl_port");
            if (tmp.length()) 
                port = tmp;
        }
        

        int portNum = atoi(port.c_str());
        m_Server = boost::make_shared<dglnet::Server>(portNum, this);
        
        std::string semaphore = Os::getEnv("dgl_semaphore");
        if (semaphore.length()) {

            //this is a rather dirty WA for local debugging
            //we fire given semaphore, when we are ready for connection (now!)

            boost::interprocess::named_semaphore sem(boost::interprocess::open_only, semaphore.c_str());
            sem.post();

        }

        m_presenter = boost::shared_ptr<OsStatusPresenter>(Os::createStatusPresenter());
        m_presenter->setStatus(Os::getProcessName() + ": wating for debugger on port " + port + ".");
                        
        m_Server->accept();
        
        dglnet::HelloMessage hello(Os::getProcessName());

        m_Server->sendMessage(&hello);

        m_presenter->setStatus(Os::getProcessName() + ": debugger connected.");

    }
    return *m_Server;
}

void DGLDebugController::run_one() {
    getServer().run_one();
    if (m_Disconnected) {
        tearDown();
    }
}

void DGLDebugController::poll() {
    getServer().poll();
    if (m_Disconnected) {
        tearDown();        
    }
}

void DGLDebugController::tearDown() {
    //it is better to die here, than allow app to run uncontrolled.

    m_presenter->setStatus(Os::getProcessName() + ": terminating");

    //this destroys "this"!
    _g_Controller.reset();
    Os::terminate();
}


BreakState& DGLDebugController::getBreakState() {
    return m_BreakState;
}

CallHistory& DGLDebugController::getCallHistory() {
    return m_CallHistory;
}

void DGLDebugController::doHandle(const dglnet::ConfigurationMessage& msg) {
    g_Config =  msg.m_config;
}
void DGLDebugController::doHandle(const dglnet::ContinueBreakMessage& msg) {
    m_BreakState.handle(msg);
}

void DGLDebugController::doHandle(const dglnet::QueryCallTraceMessage& msg) {
    dglnet::CallTraceMessage reply;
    m_CallHistory.query(msg, reply);
    m_Server->sendMessage(&reply);
}

void DGLDebugController::doHandle(const dglnet::QueryResourceMessage& msg) {
    dglState::GLContext* ctx = gc;
    for (size_t i = 0; i <  msg.m_ResourceQueries.size(); i++) {
        boost::shared_ptr<DGLResource> res;

        dglnet::ResourceMessage reply;

        try {
            if (!ctx) {
                throw std::runtime_error("No OpenGL Context present, cannot issue query");
            }
            if (msg.m_ResourceQueries[i].m_ObjectName.m_Context && 
                ctx->getId() != msg.m_ResourceQueries[i].m_ObjectName.m_Context) {
                throw std::runtime_error("Object's parent context is not current now, cannot issue query");
            }
            ctx->startQuery();
            switch (msg.m_ResourceQueries[i].m_Type) {
                case DGLResource::ObjectTypeBuffer:
                    res = ctx->queryBuffer(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeFramebuffer:
                    res = ctx->queryFramebuffer(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeFBO:
                    res = ctx->queryFBO(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeTexture:
                    res = ctx->queryTexture(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeShader:
                    res = ctx->queryShader(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeProgram:
                    res = ctx->queryProgram(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeGPU:
                    res = ctx->queryGPU(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                case DGLResource::ObjectTypeState:
                    res = ctx->queryState(msg.m_ResourceQueries[i].m_ObjectName.m_Name);
                    break;
                default:
                    throw std::runtime_error("Invalid object type requested");
            }
        } catch (const std::runtime_error& message) {
            reply.error(message.what());
        }
        std::string message;
        if (ctx && !ctx->endQuery(message)) {
            reply.error(message);
        }
        reply.m_ListenerId = msg.m_ResourceQueries[i].m_ListenerId;
        reply.m_Resource = res;
        m_Server->sendMessage(&reply);
    }
        
}

void DGLDebugController::doHandle(const dglnet::SetBreakPointsMessage& msg) {
    getBreakState().handle(msg);
}

void DGLDebugController::doHandle(const dglnet::EditShaderSourceMessage& msg) {
    dglState::GLContext* ctx = gc;
    try {
        if (!ctx) {
            throw std::runtime_error("No OpenGL Context present, cannot issue query");
        }
        if (ctx->getId() != msg.m_Context) {
                throw std::runtime_error("Object's parent context is not current now, cannot issue  query");
        }
        ctx->startQuery();

        ctx->editShaderSource(msg.m_ShaderId, msg.m_Source);

    } catch (const std::runtime_error& message) {
        assert(!"Errors from DGLDebugController::doHandle(const dglnet::EditShaderSourceMessage&) not implemented");
    }
    std::string message;
    if (ctx && !ctx->endQuery(message)) {
        assert("!Errors from DGLDebugController::doHandle(const dglnet::EditShaderSourceMessage&) not implemented");
    }
}