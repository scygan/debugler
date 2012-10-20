#include <DGLNet/server.h>

#include<map>
//#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>



typedef int NativeContextID;

class GLContext {

};


extern boost::shared_ptr<dglnet::Server> g_Server;

extern std::map<NativeContextID, boost::shared_ptr<GLContext> > g_Contexts;

extern boost::thread_specific_ptr<GLContext> g_Context;


class BreakState {
public:
    BreakState();
    bool isBreaked();
private:
    bool m_break;
};

extern BreakState g_BreakState;