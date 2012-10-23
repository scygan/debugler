#include "message.h"

namespace dglnet {

    void MessageHandler::doHandle(const BreakedCallMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const ContinueBreakMessage&) {
        unsupported();
    }

    void MessageHandler::unsupported() {
        throw std::runtime_error("Message cannot be handled by current messge handler object.");
    }

    bool ContinueBreakMessage::isBreaked() const  {
        return m_Breaked;
    }

    bool ContinueBreakMessage::isJustOneStep() const  {
        return !m_Breaked && m_JustOneStep;
    }

};
