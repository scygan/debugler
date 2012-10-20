#include"debugger.h"


boost::shared_ptr<dglnet::Server> g_Server;

std::map<NativeContextID, boost::shared_ptr<GLContext> > g_contexts;

boost::thread_specific_ptr<GLContext> g_context;

BreakState g_BreakState;


BreakState::BreakState():m_break(false) {}

bool BreakState::isBreaked() {
    return m_break;
}

