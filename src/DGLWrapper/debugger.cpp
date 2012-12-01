#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

#include"debugger.h"

boost::shared_ptr<DebugController> g_Controller;
GLState g_GLState;
DGLConfiguration g_Config;

template<typename T>
void nop(T*) {}

GLState::GLState():m_Current(&nop<dglstate::GLContext>), m_CurrentSurface(&nop<dglstate::NPISurface>) {};

GLState::ContextListIter GLState::ensureContext(uint32_t id, bool lock) {
    if (lock) {
        m_ContextListMutex.lock();
    }
    ContextListIter i = m_ContextList.find(id);
    if (i == m_ContextList.end()) {
        i = m_ContextList.insert(
                std::pair<uint32_t, boost::shared_ptr<dglstate::GLContext>> (
                    id, boost::make_shared<dglstate::GLContext>(id)
                    )
            ).first;
    }
    if (lock) {
        m_ContextListMutex.unlock();
    }
    return i;
}

GLState::SurfaceListIter GLState::ensureSurface(uint32_t id, bool lock) {
    if (lock) {
        m_SurfaceListMutex.lock();
    }
    SurfaceListIter i = m_SurfaceList.find(id);
    if (i == m_SurfaceList.end()) {
        i = m_SurfaceList.insert(
            std::pair<uint32_t, boost::shared_ptr<dglstate::NPISurface>> (
            id, boost::make_shared<dglstate::NPISurface>(id)
            )
            ).first;
    }
    if (lock) {
        m_SurfaceListMutex.unlock();
    }
    return i;
}

dglstate::GLContext* GLState::getCurrent() {
    return m_Current.get();
}

void GLState::bindContext(uint32_t ctxId, uint32_t npiSurfaceId) {
    dglstate::GLContext* current = getCurrent();

    if (current && ctxId == current->getId())
        return;
    
    if (current) {
        current->use(false);
        if (current->isDeleted()) {
            deleteContext(current->getId());
        }
    }
    if (ctxId) {
        m_Current.reset(&(*(ensureContext(ctxId)->second)));
        getCurrent()->use(true);
        
        dglstate::NPISurface* npiSurface = getCurrent()->getNpiSurface();
        if (!npiSurface || npiSurface->getId() != npiSurfaceId) {
            getCurrent()->setNpiSurface(&(*(ensureSurface(npiSurfaceId)->second)));
        }

    } else {
        m_Current.release();
    }
}

void GLState::deleteContext(uint32_t id) {
    boost::lock_guard<boost::mutex> quard(m_ContextListMutex);
    ContextListIter i = ensureContext(id, false);
    if (i->second->lazyDelete()) {
        m_ContextList.erase(i);
    }
}

std::vector<dglnet::ContextReport> GLState::describe() {
    boost::lock_guard<boost::mutex> quard(m_ContextListMutex);
    std::vector<dglnet::ContextReport> ret(m_ContextList.size());
    int j = 0;
    for (ContextListIter i = m_ContextList.begin(); i != m_ContextList.end(); i++) {
        ret[j++] = i->second->describe();
    }
    return ret;
}

BreakState::BreakState():m_break(true),m_StepModeEnabled(false) {}

bool BreakState::breakAt(const Entrypoint& e, GLenum error) {
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
    if (error != GL_NO_ERROR) {
        m_break = true;
    }

    return isBreaked();
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
    size_t endOffset = min(query.m_EndOffset,m_cb.size());

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

DebugController::~DebugController() {
    if (m_Server) {
        m_Server->abort();
        m_Server.reset();
    }
}

void DebugController::connect(boost::shared_ptr<dglnet::Server> srv) {
    m_Server = srv;
}

void DebugController::doHandleDisconnect(const std::string&) {
    //we have got disconnected from the client
    //it is better to die here, than allow app to run uncontrolled.

    //this destroys "this"!
    g_Controller.reset();
    TerminateProcess(GetCurrentProcess(), 0);
}

dglnet::Server& DebugController::getServer() {
    return *m_Server;
}

BreakState& DebugController::getBreakState() {
    return m_BreakState;
}

CallHistory& DebugController::getCallHistory() {
    return m_CallHistory;
}

void DebugController::doHandle(const dglnet::ConfigurationMessage& msg) {
    g_Config.m_BreakOnGLError = msg.m_BreakOnGLError;
}

void DebugController::doHandle(const dglnet::ContinueBreakMessage& msg) {
    m_BreakState.handle(msg);
}

void DebugController::doHandle(const dglnet::QueryCallTraceMessage& msg) {
    dglnet::CallTraceMessage reply;
    m_CallHistory.query(msg, reply);
    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::QueryTextureMessage& msg) {
    dglnet::TextureMessage reply;
    dglstate::GLContext* ctx = g_GLState.getCurrent();
    if (ctx) {
        ctx->queryTexture(msg.m_TextureName, reply);
    }
    
    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::QueryBufferMessage& msg) {
    dglnet::BufferMessage reply;
    dglstate::GLContext* ctx = g_GLState.getCurrent();
    if (ctx) {
        ctx->queryBuffer(msg.m_BufferName, reply);
    }

    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::QueryFramebufferMessage& msg) {
    dglnet::FramebufferMessage reply;
    dglstate::GLContext* ctx = g_GLState.getCurrent();
    if (ctx) {
        ctx->queryFramebuffer(msg.m_BufferEnum, reply);
    }

    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::QueryFBOMessage& msg) {
    dglnet::FBOMessage reply;
    dglstate::GLContext* ctx = g_GLState.getCurrent();
    if (ctx) {
        ctx->queryFBO(msg.m_Name, reply);
    }

    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::QueryShaderMessage& msg) {
    dglnet::ShaderMessage reply;
    dglstate::GLContext* ctx = g_GLState.getCurrent();
    if (ctx) {
        ctx->queryShader(msg.m_Name, reply);
    }

    m_Server->sendMessage(&reply);
}

void DebugController::doHandle(const dglnet::SetBreakPointsMessage& msg) {
    getBreakState().handle(msg);
}