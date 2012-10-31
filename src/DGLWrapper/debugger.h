#include <DGLNet/server.h>

#include<map>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>

#define CALL_HISTORY_LEN 1000


typedef int NativeContextID;

class GLContext {

};


class BreakState {
public:
    BreakState();
    void handle(const dglnet::ContinueBreakMessage&);
    bool isBreaked();
    void endStep();
private:
    bool m_break;
    bool m_isJustOneStep;
};

class CallHistory {
public:
    CallHistory();
    void add(const CalledEntryPoint&);
    void query( const dglnet::QueryCallTraceMessage& query, dglnet::CallTraceMessage& reply);
    size_t size();
private:
    boost::circular_buffer<CalledEntryPoint> m_cb;
    boost::mutex m_mutex;
};

class DebugController: public dglnet::MessageHandler {
public:
    void connect(boost::shared_ptr<dglnet::Server>);
    virtual void doHandleDisconnect(const std::string&);

    BreakState m_BreakState;

    dglnet::Server& getServer();
    BreakState& getBreakState();
    CallHistory& getCallHistory();

    //Message handlers
    void doHandle(const dglnet::ContinueBreakMessage&);
    void doHandle(const dglnet::QueryCallTraceMessage&);

private:
    boost::shared_ptr<dglnet::Server> m_Server;
    CallHistory m_CallHistory;
};


extern boost::shared_ptr<DebugController> g_Controller;

extern std::map<NativeContextID, boost::shared_ptr<GLContext> > g_Contexts;

extern boost::thread_specific_ptr<GLContext> g_Context;
