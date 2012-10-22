#include"debugger.h"


boost::shared_ptr<DebugController> g_Controller;

std::map<NativeContextID, boost::shared_ptr<GLContext> > g_contexts;

boost::thread_specific_ptr<GLContext> g_context;


BreakState::BreakState():m_break(true),m_isStep(false) {}

bool BreakState::isBreaked() {
    return m_break;
}

void BreakState::continueStep() {
    m_break = false; m_isStep = true;
}

void BreakState::endStep() {
    if (m_isStep) {
        m_break = true; m_isStep = false;
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


void DebugController::doHandle(const dglnet::DebugStepMessage&) {
    getBreakState().continueStep();
}