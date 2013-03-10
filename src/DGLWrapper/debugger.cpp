#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

#include"debugger.h"
#include <boost/interprocess/sync/named_semaphore.hpp>

boost::shared_ptr<DGLDebugController> _g_Controller;

DGLDebugController* getController() {
    if (!_g_Controller.get()) {
        _g_Controller = boost::make_shared<DGLDebugController>();
    }
    return _g_Controller.get();
};



DGLGLState g_DGLGLState;
DGLConfiguration g_Config;

template<typename T>
void nop(T*) {}

DGLGLState::DGLGLState():m_Current(&nop<dglState::GLContext>), m_CurrentSurface(&nop<dglState::NativeSurface>) {};

DGLGLState::ContextListIter DGLGLState::ensureContext(uint32_t id, bool lock) {
    if (lock) {
        m_ContextListMutex.lock();
    }
    ContextListIter i = m_ContextList.find(id);
    if (i == m_ContextList.end()) {
        i = m_ContextList.insert(
                std::pair<uint32_t, boost::shared_ptr<dglState::GLContext> > (
                    id, boost::make_shared<dglState::GLContext>(id)
                    )
            ).first;
    }
    if (lock) {
        m_ContextListMutex.unlock();
    }
    return i;
}

DGLGLState::SurfaceListIter DGLGLState::ensureSurface(uint32_t id, bool lock) {
    if (lock) {
        m_SurfaceListMutex.lock();
    }
    SurfaceListIter i = m_SurfaceList.find(id);
    if (i == m_SurfaceList.end()) {
        i = m_SurfaceList.insert(
            std::pair<uint32_t, boost::shared_ptr<dglState::NativeSurface> > (
            id, boost::make_shared<dglState::NativeSurface>(id)
            )
            ).first;
    }
    if (lock) {
        m_SurfaceListMutex.unlock();
    }
    return i;
}

dglState::GLContext* DGLGLState::getCurrent() {
    return m_Current.get();
}

void DGLGLState::bindContext(uint32_t ctxId, uint32_t NativeSurfaceId) {
    dglState::GLContext* current = getCurrent();

    if (current && ctxId == current->getId())
        return;
    
    if (ctxId) {
        m_Current.reset(&(*(ensureContext(ctxId)->second)));
        
        dglState::NativeSurface* NativeSurface = getCurrent()->getNativeSurface();
        if (!NativeSurface || NativeSurface->getId() != NativeSurfaceId) {
            getCurrent()->setNativeSurface(&(*(ensureSurface(NativeSurfaceId)->second)));
        }

        getCurrent()->bound();

    } else {
        m_Current.release();
    }
}

void DGLGLState::deleteContext(uint32_t id) {
    boost::lock_guard<boost::mutex> quard(m_ContextListMutex);
    if (getCurrent() && getCurrent()->getId() == id) {
        bindContext(0, 0);
    }
    m_ContextList.erase(id);
 }

std::vector<dglnet::ContextReport> DGLGLState::describe() {
    boost::lock_guard<boost::mutex> quard(m_ContextListMutex);
    std::vector<dglnet::ContextReport> ret(m_ContextList.size());
    int j = 0;
    for (ContextListIter i = m_ContextList.begin(); i != m_ContextList.end(); i++) {
        ret[j++] = i->second->describe();
    }
    return ret;
}

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
    size_t endOffset = std::min((size_t)query.m_EndOffset,m_cb.size());

    boost::circular_buffer<CalledEntryPoint>::iterator begin, end;
    end = m_cb.end() - startOffset;
    begin = m_cb.end() - endOffset;
    std::back_insert_iterator<std::vector<CalledEntryPoint> > replyHistory(reply.m_Trace);
    std::copy(begin, end, replyHistory);
}

size_t CallHistory::size() {
    return m_cb.size();
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
    dglState::GLContext* ctx = g_DGLGLState.getCurrent();
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
