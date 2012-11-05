#include <DGLNet/server.h>

#include<map>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>

#define CALL_HISTORY_LEN 1000


class GLObj {};

class GLBufferObj: public GLObj {};

class GLTextureObj: public GLObj {};

class GLProgramObj: public GLObj {};

class GLShaderObj: public GLObj {};


class GLContext {
public:
    GLContext(uint32_t id);
    bool m_deleted;
    std::map<GLuint, GLTextureObj> m_Textures;
    std::map<GLuint, GLBufferObj> m_Buffers;
    
    dglnet::ContextReport describe();

    void use(bool);
    bool lazyDelete();
    bool isDeleted();
    
    int32_t getId();

private:
    int32_t m_Id;
    bool m_InUse, m_Deleted;
};


class GLState {
    typedef std::map<uint32_t, boost::shared_ptr<GLContext> >::iterator ContextListIter;
public:
    GLContext* getCurrent();
    ContextListIter ensureContext(uint32_t id, bool lock = true);
    void bindContext(uint32_t id);
    void deleteContext(uint32_t id);

    std::vector<dglnet::ContextReport> describe();

private:
    std::map<uint32_t, boost::shared_ptr<GLContext> > m_ContextList;
    boost::thread_specific_ptr<GLContext> m_Current;
    boost::mutex m_ContextListMutex;
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
    void query(const dglnet::QueryCallTraceMessage& query, dglnet::CallTraceMessage& reply);
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
extern GLState g_GLState; 