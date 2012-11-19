#include <DGLNet/server.h>

#include<map>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>

#include "gl-state.h"

#define CALL_HISTORY_LEN 1000


class GLState {
    typedef std::map<uint32_t, boost::shared_ptr<dglstate::GLContext> >::iterator ContextListIter;
    typedef std::map<uint32_t, boost::shared_ptr<dglstate::NPISurface> >::iterator SurfaceListIter;
public:
    GLState();
    dglstate::GLContext* getCurrent();
    ContextListIter ensureContext(uint32_t id, bool lock = true);
    SurfaceListIter ensureSurface(uint32_t id, bool lock = true);
    void bindContext(uint32_t id, uint32_t hdc);
    void deleteContext(uint32_t id);

    std::vector<dglnet::ContextReport> describe();

private:
    std::map<uint32_t, boost::shared_ptr<dglstate::GLContext> > m_ContextList;
    std::map<uint32_t, boost::shared_ptr<dglstate::NPISurface> > m_SurfaceList;
    boost::thread_specific_ptr<dglstate::GLContext> m_Current;
    boost::thread_specific_ptr<dglstate::NPISurface> m_CurrentSurface;

    boost::mutex m_ContextListMutex;
    boost::mutex m_SurfaceListMutex;
};

class BreakState {
public:
    BreakState();
    bool breakAt(const Entrypoint&);
    bool isBreaked();
    void endStep();
    //handlers for remote commands
    void handle(const dglnet::ContinueBreakMessage&);
    void handle(const dglnet::SetBreakPointsMessage&);
private:
    bool m_break;
    bool m_isJustOneStep;
    std::set<Entrypoint> m_BreakPoints;
};

class CallHistory {
public:
    CallHistory();
    void add(const CalledEntryPoint&);
    void query(const dglnet::QueryCallTraceMessage& query, dglnet::CallTraceMessage& reply);
    size_t size();
private:
    boost::circular_buffer<CalledEntryPoint> m_cb;
    boost::mutex m_mutex;
};

class DebugController: public dglnet::MessageHandler {
public:
    ~DebugController();
    void connect(boost::shared_ptr<dglnet::Server>);
    virtual void doHandleDisconnect(const std::string&);

    BreakState m_BreakState;

    dglnet::Server& getServer();
    BreakState& getBreakState();
    CallHistory& getCallHistory();

    //Message handlers
    void doHandle(const dglnet::ContinueBreakMessage&);
    void doHandle(const dglnet::QueryCallTraceMessage&);
    void doHandle(const dglnet::QueryTextureMessage&);
    void doHandle(const dglnet::QueryBufferMessage&);
    void doHandle(const dglnet::QueryFramebufferMessage&);
    void doHandle(const dglnet::QueryFBOMessage&);
    void doHandle(const dglnet::SetBreakPointsMessage&);

private:
    boost::shared_ptr<dglnet::Server> m_Server;
    CallHistory m_CallHistory;
};

extern boost::shared_ptr<DebugController> g_Controller;
extern GLState g_GLState; 