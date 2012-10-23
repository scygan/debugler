#include"debugger.h"


boost::shared_ptr<DebugController> g_Controller;

std::map<NativeContextID, boost::shared_ptr<GLContext> > g_contexts;

boost::thread_specific_ptr<GLContext> g_context;


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



void DebugController::connect(boost::shared_ptr<dglnet::Server> srv) {
    m_Server = srv;
}

dglnet::Server& DebugController::getServer() {
    return *m_Server;
}

BreakState& DebugController::getBreakState() {
    return m_BreakState;
}


void DebugController::doHandle(const dglnet::ContinueBreakMessage& msg) {
    m_BreakState.handle(msg);
}