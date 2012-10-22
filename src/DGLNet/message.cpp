#include "message.h"

namespace dglnet {

    void MessageHandler::doHandle(const BreakedCallMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const DebugStepMessage&) {
        unsupported();
    }

    void MessageHandler::unsupported() {
        throw std::runtime_error("Messag7e cannot be handled by current messge handler object.");
    }
};
