#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

#include"debugger.h"

boost::shared_ptr<DebugController> g_Controller;
GLState g_GLState;

GLContext::GLContext(uint32_t id):m_Id(id), m_InUse(false), m_Deleted(false) {}

dglnet::ContextReport GLContext::describe() {
    dglnet::ContextReport ret(m_Id);
    for (std::map<GLuint, GLTextureObj>::iterator i = m_Textures.begin(); 
        i != m_Textures.end(); i++) {
            ret.m_TextureSpace.insert(i->second.getName());
    }
    for (std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.begin(); 
        i != m_Buffers.end(); i++) {
            ret.m_BufferSpace.insert(i->second.getName());
    }
    return ret;
}

void GLContext::use(bool inUse) {
    m_InUse = inUse;
}

bool GLContext::lazyDelete() {
    m_Deleted = true;
    return !m_InUse;
}

bool GLContext::isDeleted() {
    return m_Deleted;
}

void GLContext::ensureTexture(GLuint name) {
    if (m_Textures.find(name) == m_Textures.end())
        m_Textures[name] = GLTextureObj(name);
}

void GLContext::deleteTexture(GLuint name) {
    std::map<GLuint, GLTextureObj>::iterator i = m_Textures.find(name); 
    if (i !=  m_Textures.end()) {
        m_Textures.erase(i);
    }
}

void GLContext::ensureBuffer(GLuint name) {
    if (m_Buffers.find(name) == m_Buffers.end())
        m_Buffers[name] = GLBufferObj(name);
}

void GLContext::deleteBuffer(GLuint name) {
    std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.find(name); 
    if (i !=  m_Buffers.end()) {
        m_Buffers.erase(i);
    }
}

int32_t GLContext::getId() {
    return m_Id;
}

void nop(GLContext*) {}

GLState::GLState():m_Current(&nop) {};

GLState::ContextListIter GLState::ensureContext(uint32_t id, bool lock) {
    if (lock) {
        m_ContextListMutex.lock();
    }
    ContextListIter i = m_ContextList.find(id);
    if (i == m_ContextList.end()) {
        i = m_ContextList.insert(
                std::pair<uint32_t, boost::shared_ptr<GLContext>> (
                    id, boost::make_shared<GLContext>(id)
                    )
            ).first;
    }
    if (lock) {
        m_ContextListMutex.unlock();
    }
    return i;
}

GLContext* GLState::getCurrent() {
    return m_Current.get();
}

void GLState::bindContext(uint32_t id) {
    GLContext* current = getCurrent();

    if (current && id == current->getId())
        return;
    
    if (current) {
        current->use(false);
        if (current->isDeleted()) {
            deleteContext(current->getId());
        }
    }
    if (id) {
        m_Current.reset(&(*(ensureContext(id)->second)));
        getCurrent()->use(true);
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

BreakState::BreakState():m_break(true),m_isJustOneStep(false) {}

bool BreakState::isBreaked() {
    return m_break;
}

void BreakState::handle(const dglnet::ContinueBreakMessage& msg) {
    m_break = msg.isBreaked();
    if (!m_break) {
        m_isJustOneStep = msg.isJustOneStep();
    } else {
        m_isJustOneStep = false;
    }
}

void BreakState::endStep() {
    if (m_isJustOneStep) {
        m_break = true; m_isJustOneStep = false;
    }
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

void DebugController::connect(boost::shared_ptr<dglnet::Server> srv) {
    m_Server = srv;
}

void DebugController::doHandleDisconnect(const std::string&) {
    //we have got disconnected from the client
    //it is better to die here, than allow app to run uncontrolled.
    exit(1);
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


void DebugController::doHandle(const dglnet::ContinueBreakMessage& msg) {
    m_BreakState.handle(msg);
}

void DebugController::doHandle(const dglnet::QueryCallTraceMessage& msg) {
    dglnet::CallTraceMessage reply;
    m_CallHistory.query(msg, reply);
    m_Server->sendMessage(&reply);
}