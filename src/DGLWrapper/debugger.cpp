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

#include "debugger.h"

#include "native-surface.h"
#include "tls.h"
#include "ipc.h"

#include <DGLNet/server.h>

#include <sstream>
#include <boost/make_shared.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>

std::shared_ptr<DGLDebugController> _g_Controller;

DGLDebugController* getController() {
    if (!_g_Controller.get()) {
        _g_Controller = std::make_shared<DGLDebugController>();
    }
    return _g_Controller.get();
}

DGLConfiguration g_Config;

BreakState::BreakState()
        : m_break(true), //always give initial break
          m_BreakingEnabled(false), //initially all breaks are masked until connection is made.
          m_StepModeEnabled(false),
          m_StepMode(dglnet::message::ContinueBreak::StepMode::CALL) {}

void BreakState::setEnabled(bool enabled) {
    m_BreakingEnabled = enabled;
}

bool BreakState::mayBreakAt(const Entrypoint& e) {
    if (m_StepModeEnabled) {
        switch (m_StepMode) {
            case dglnet::message::ContinueBreak::StepMode::CALL:
                setBreak();
                break;
            case dglnet::message::ContinueBreak::StepMode::DRAW_CALL:
                if (IsDrawCall(e)) setBreak();
                break;
            case dglnet::message::ContinueBreak::StepMode::FRAME:
                if (IsFrameDelimiter(e)) setBreak();
                break;
        }
    }

    if (m_BreakPoints.find(e) != m_BreakPoints.end()) {
        setBreak();
    }
    return isBreaked();
}

void BreakState::setBreakAtGLError(GLenum glError) {
    if (glError != GL_NO_ERROR && g_Config.m_BreakOnGLError) {
        setBreak();
    }
}

void BreakState::setBreakAtDebugOutput() {
    if (g_Config.m_BreakOnDebugOutput) setBreak();
}

void BreakState::setBreakAtCompilerError() {
    if (g_Config.m_BreakOnCompilerError) setBreak();
}

bool BreakState::isBreaked() { return m_break && m_BreakingEnabled; }

void BreakState::handle(const dglnet::message::ContinueBreak& msg) {
    m_break = msg.isBreaked();
    if (!m_break) {
        if ((m_StepModeEnabled = msg.getStep().first) != false) {
            m_StepMode = msg.getStep().second;
        }
    }
}

void BreakState::handle(const dglnet::message::SetBreakPoints& msg) {
    m_BreakPoints = msg.get();
}

void BreakState::setBreak(bool _break) {
    //mask all changes to m_break by m_BreakingEnabled
    //If apps run with braking disabled, m_break will not be modified.
    if (m_BreakingEnabled) {
        m_break = _break;
    }
}

CallHistory::CallHistory() {
    m_cb = boost::circular_buffer<CalledEntryPoint>(CALL_HISTORY_LEN);
}

void CallHistory::add(const CalledEntryPoint& entryp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cb.push_back(entryp);
}

void CallHistory::query(const dglnet::message::QueryCallTrace& traceQuery,
                        dglnet::message::CallTrace& reply) {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t startOffset = reply.m_StartOffset = traceQuery.m_StartOffset;
    if (startOffset >= m_cb.size()) return;    // queried non existent elements

    // trim query to sane range:
    size_t endOffset =
            std::min(static_cast<size_t>(traceQuery.m_EndOffset), m_cb.size());

    boost::circular_buffer<CalledEntryPoint>::iterator begin, end;
    end = m_cb.end() - startOffset;
    begin = m_cb.end() - endOffset;
    std::back_insert_iterator<std::vector<CalledEntryPoint> > replyHistory(
            reply.m_Trace);
    std::copy(begin, end, replyHistory);
}

size_t CallHistory::size() { return m_cb.size(); }

void CallHistory::setRetVal(const RetValue& ret) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cb.back().setRetVal(ret);
}

void CallHistory::setError(GLenum error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cb.back().setError(error);
}

void CallHistory::setDebugOutput(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cb.back().setDebugOutput(message);
}

DGLDebugServer::DGLDebugServer(DGLDebugController* parrent)
        : m_parrent(parrent) {}

DGLDebugServer::~DGLDebugServer() {
    //getMutex().unlock();
}

std::mutex& DGLDebugServer::getMutex() { return m_ServerMutex; }

std::shared_ptr<dglnet::ITransport> DGLDebugServer::getTransport() {
    return m_Transport;
}

template <class server_type>
void DGLDebugServer::listen(const std::string& port, bool wait) {
    std::shared_ptr<server_type> server =
            std::make_shared<server_type>(port, m_parrent);
    m_Transport = std::static_pointer_cast<dglnet::ITransport>(server);

    m_parrent->doHandleListen(port);

    server->accept(wait);
}

void DGLDebugServer::abort() {
    if (m_Transport) {
        m_Transport->abort();
        m_Transport.reset();
    }
}

DGLDebugController::DGLDebugController()
        : m_BreakState(),
          m_Disconnected(false),
          m_Server(this) {}

DGLDebugController::~DGLDebugController() { m_Server.abort(); }

void DGLDebugController::doHandleListen(const std::string& port) {
    std::string semaphore = Os::getEnv("dgl_semaphore");

    if (semaphore.length()) {
        // this is a rather dirty WA for local debugging
        // we fire given semaphore, when we are ready for connection (now!)

        boost::interprocess::named_semaphore sem(boost::interprocess::open_only,
                                                 semaphore.c_str());
        sem.post();
    }
    m_presenter =
            std::shared_ptr<OsStatusPresenter>(Os::createStatusPresenter());
    {
        std::ostringstream msg;
        msg << Os::getProcessName() << ": waiting for debugger on port " << port
            << ".";
        m_presenter->setStatus(msg.str());
    }
}

void DGLDebugController::doHandleConnect() {

    getBreakState().setEnabled(true);

    dglnet::message::Hello hello(Os::getProcessName());

    getServer().getTransport()->sendMessage(&hello);

    m_presenter->setStatus(Os::getProcessName() + ": debugger connected.");
}

void DGLDebugController::doHandleDisconnect(const std::string&) {
    // we have got disconnected from the client. Mark this in controller state
    // and return, so io_service can be freed later
    m_Disconnected = true;
}

DGLDebugServer& DGLDebugController::getServer() {
    int pid;

    if (m_LastPid != (pid = Os::getProcessPid())) {
        //fork occured. Reset connection.
        m_Server.abort();
        m_LastPid = pid;
    }
    if (!m_Server.getTransport()) {
        std::string port;
        DGLIPC::DebuggerPortType portType = getIPC()->getDebuggerPort(port);

        {
            size_t pos = 0;
            while ((pos = port.find("%n")) != std::string::npos) {
                port.replace(pos, 2, Os::getProcessName());
            }
        }
        {
            size_t pos = 0;
            while ((pos = port.find("%p")) != std::string::npos) {
                std::ostringstream pidStr;
                pidStr << pid;
                port.replace(pos, 2, pidStr.str());
            }
        }

        Os::info("port = %s", port.c_str());

        //We should wait for actual connection when: 
        // - no -nowait was specified in dglloader (so application runs freely)
        // - execution is breaked. (It requires connection if breaked).
        // - on Android, when explicitely requested by property variable.
        //Otherwise we set socket in listening state and continue.
        bool wait = getIPC()->getWaitForConnection() || getBreakState().isBreaked();

#ifdef __ANDROID__
        {
            //get global breakpoint
            std::string processBreakPoint = Os::getProp("debug." DGL_PRODUCT_LOWER ".break");

            std::string processName = Os::getProcessName();

            bool droidWait = (processBreakPoint == processName);

            Os::info("Process name: %s, process breakpoint: %s, waiting for debugger: %d", 
                processName.c_str(), processBreakPoint.c_str(), (int)droidWait);

            wait |= droidWait;
        }
#endif

        switch (portType) {
            case DGLIPC::DebuggerPortType::TCP:
                m_Server.listen<dglnet::ServerTcp>(port, wait);
                break;

            case DGLIPC::DebuggerPortType::UNIX:

#ifndef _WIN32
                unlink(port.c_str());
                m_Server.listen<dglnet::ServerUnixDomain>(port, wait);
#else
                throw std::runtime_error(
                        "Unix sockets are not supported on Windows.");
#endif
                break;
            default:
                assert(0);
        }
    }
    return m_Server;
}

void DGLDebugController::run_one(bool& newConnection) {
    getServer().getTransport()->run_one();
    
    newConnection = m_Disconnected;

    if (m_Disconnected) {
        onConnectionLost();
    }
}

void DGLDebugController::poll() {
    getServer().getTransport()->poll();

    if (m_Disconnected) {
        onConnectionLost();
    }
}

void DGLDebugController::onConnectionLost() {
    
    //recover, wait for new connection

    m_Disconnected = false;

    m_presenter->setStatus(Os::getProcessName() + ": connection lost");
    
    m_Server.abort();

    //Disable breaks.
    // So, for example, -nowait application will never break if not connected
    getBreakState().setEnabled(false);

    //now continue executing action code. Someone will eventually 
    //call getServer() now and connection will be re-estabilished.
}

BreakState& DGLDebugController::getBreakState() { return m_BreakState; }

CallHistory& DGLDebugController::getCallHistory() { return m_CallHistory; }

void DGLDebugController::doHandleConfiguration(
        const dglnet::message::Configuration& msg) {
    g_Config = msg.m_config;
}
void DGLDebugController::doHandleContinueBreak(
        const dglnet::message::ContinueBreak& msg) {
    m_BreakState.handle(msg);
}

void DGLDebugController::doHandleTerminate(
    const dglnet::message::Terminate&) {

    //Exiting here would cause locked mutexes and dead thread owning them problem. 
    //So throw, and exit few frames higher, where no locks exist.
    throw TeardownException();
}

void DGLDebugController::doHandleQueryCallTrace(
        const dglnet::message::QueryCallTrace& msg) {
    dglnet::message::CallTrace reply;
    m_CallHistory.query(msg, reply);
    getServer().getTransport()->sendMessage(&reply);
}

void DGLDebugController::doHandleSetBreakPoints(
        const dglnet::message::SetBreakPoints& msg) {
    getBreakState().handle(msg);
}

void DGLDebugController::doHandleRequest(const dglnet::message::Request& msg) {
    dglnet::message::RequestReply reply;

    try {
        // only one request type for now
        if (dynamic_cast<const dglnet::request::QueryResource*>(
                    msg.m_Request.get())) {
            reply.m_Reply = doHandleRequest(
                    *dynamic_cast<const dglnet::request::QueryResource*>(
                             msg.m_Request.get()));
        } else if (dynamic_cast<const dglnet::request::EditShaderSource*>(
                           msg.m_Request.get())) {
            doHandleRequest(
                    *dynamic_cast<const dglnet::request::EditShaderSource*>(
                             msg.m_Request.get()));
        } else if (dynamic_cast<const dglnet::request::ForceLinkProgram*>(
                           msg.m_Request.get())) {
            doHandleRequest(
                    *dynamic_cast<const dglnet::request::ForceLinkProgram*>(
                             msg.m_Request.get()));
        } else {
            reply.error("Cannot handle: unsupported request");
        }
    }
    catch (const std::runtime_error& message) {
        reply.error(message.what());
    }

    reply.m_RequestId = msg.getId();
    getServer().getTransport()->sendMessage(&reply);
}

boost::shared_ptr<dglnet::DGLResource> DGLDebugController::doHandleRequest(
        const dglnet::request::QueryResource& request) {

    boost::shared_ptr<dglnet::DGLResource> resource;

    dglState::GLContext* ctx = gc;

    if (!ctx) {
        throw std::runtime_error(
                "No OpenGL Context present, cannot issue query");
    }
    if (request.m_ObjectName.m_Context &&
        ctx->getId() != request.m_ObjectName.m_Context) {
        throw std::runtime_error(
                "Object's parent context is not current now, cannot issue "
                "query");
    }
    ctx->startQuery();
    switch (request.m_Type) {
        case dglnet::DGLResource::ObjectType::Buffer:
            resource = ctx->queryBuffer(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::Framebuffer:
            resource = ctx->queryFramebuffer(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::FBO:
            resource = ctx->queryFBO(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::Texture:
            resource = ctx->queryTexture(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::Shader:
            resource = ctx->queryShader(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::Program:
            resource = ctx->queryProgram(request.m_ObjectName.m_Name);
            break;
        case dglnet::DGLResource::ObjectType::GPU:
            resource = ctx->queryGPU();
            break;
        case dglnet::DGLResource::ObjectType::State:
            resource = ctx->queryState(request.m_ObjectName.m_Name);
            break;
        default:
            throw std::runtime_error("Unsupported query type");
    }

    std::string message;
    if (ctx && !ctx->endQuery(message)) {
        throw std::runtime_error(message);
    }

    return resource;
}

void DGLDebugController::doHandleRequest(
        const dglnet::request::EditShaderSource& request) {
    dglState::GLContext* ctx = gc;
    if (!ctx) {
        throw std::runtime_error(
                "No OpenGL Context present, cannot issue request");
    }
    if (ctx->getId() != request.m_Context) {
        throw std::runtime_error(
                "Object's parent context is not current now, cannot issue  "
                "query");
    }
    ctx->startQuery();

    dglState::GLShaderObj* obj =
            ctx->findShader(static_cast<GLuint>(request.m_ShaderId));
    if (!obj) {
        throw std::runtime_error("Shader does not exist");
    }
    if (request.m_Reset) {
        obj->resetSourceToOrig();
    } else {
        obj->editSource(request.m_Source);
    }

    std::string message;
    if (ctx && !ctx->endQuery(message)) {
        throw std::runtime_error(message);
    }
}

void DGLDebugController::doHandleRequest(
        const dglnet::request::ForceLinkProgram& request) {
    dglState::GLContext* ctx = gc;
    if (!ctx) {
        throw std::runtime_error(
                "No OpenGL Context present, cannot issue request");
    }
    if (ctx->getId() != request.m_Context) {
        throw std::runtime_error(
                "Object's parent context is not current now, cannot issue  "
                "query");
    }
    ctx->startQuery();

    dglState::GLProgramObj* obj =
            ctx->findProgram(static_cast<GLuint>(request.m_ProgramId));
    if (!obj) {
        throw std::runtime_error("Program does not exist");
    }

    obj->forceLink();

    std::string message;
    if (ctx && !ctx->endQuery(message)) {
        throw std::runtime_error(message);
    }
}
