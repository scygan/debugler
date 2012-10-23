#include <DGLNet/server.h>

#include<map>
//#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>



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

class DebugController: public dglnet::MessageHandler {
public:
    void connect(boost::shared_ptr<dglnet::Server>);
    BreakState m_BreakState;

    dglnet::Server& getServer();
    BreakState& getBreakState();

    //Message handlers
    void doHandle(const dglnet::ContinueBreakMessage&);

private:
    boost::shared_ptr<dglnet::Server> m_Server;
};


extern boost::shared_ptr<DebugController> g_Controller;

extern std::map<NativeContextID, boost::shared_ptr<GLContext> > g_Contexts;

extern boost::thread_specific_ptr<GLContext> g_Context;
