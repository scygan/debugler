#include "message.h"

namespace dglnet {

    void MessageHandler::doHandle(const BreakedCallMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const ContinueBreakMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const QueryCallTraceMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const CallTraceMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const QueryTextureMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const TextureMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const SetBreakPointsMessage&) {
        unsupported();
    }

    void MessageHandler::unsupported() {
        throw std::runtime_error("Message cannot be handled by current message handler object.");
    }

    bool ContinueBreakMessage::isBreaked() const  {
        return m_Breaked;
    }

    bool ContinueBreakMessage::isJustOneStep() const  {
        return !m_Breaked && m_JustOneStep;
    }

    TextureMessage::TextureMessage():m_Ok(true), m_TextureName(0) {}

    void TextureMessage::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool TextureMessage::isOk(std::string& msg) {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    SetBreakPointsMessage::SetBreakPointsMessage(const std::set<Entrypoint>& breakpoints):m_BreakPoints(breakpoints) {}

    std::set<Entrypoint> SetBreakPointsMessage::get() const {
        return m_BreakPoints;
    }

};