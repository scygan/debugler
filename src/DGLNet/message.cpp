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

    void MessageHandler::doHandle(const QueryBufferMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const BufferMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const QueryFramebufferMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const FramebufferMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const QueryFBOMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const FBOMessage&) {
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

     std::pair<bool, ContinueBreakMessage::StepMode> ContinueBreakMessage::getStep() const  {
        return std::pair<bool, StepMode>(m_InStepMode, m_StepMode);
    }

    TextureMessage::TextureMessage():m_Ok(true), m_TextureName(0) {}

    void TextureMessage::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool TextureMessage::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    BufferMessage::BufferMessage():m_Ok(true), m_BufferName(0) {}

    void BufferMessage::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool BufferMessage::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    FramebufferMessage::FramebufferMessage():m_Ok(true), m_BufferEnum(0) {}

    void FramebufferMessage::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool FramebufferMessage::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    FBOMessage::FBOMessage():m_Ok(true), m_Name(0) {}

    FBOAttachment::FBOAttachment(uint32_t id):m_Ok(true),m_Id(id) {}

    void FBOAttachment::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool FBOAttachment::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    void FBOMessage::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool FBOMessage::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    SetBreakPointsMessage::SetBreakPointsMessage(const std::set<Entrypoint>& breakpoints):m_BreakPoints(breakpoints) {}

    std::set<Entrypoint> SetBreakPointsMessage::get() const {
        return m_BreakPoints;
    }

};