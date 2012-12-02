#include "message.h"

namespace dglnet {

    void MessageHandler::doHandle(const ConfigurationMessage&) {
        unsupported();
    }

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

    void MessageHandler::doHandle(const QueryShaderMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const ShaderMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const QueryProgramMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const ProgramMessage&) {
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

    StatusMessage::StatusMessage():m_Ok(true) {}

    void StatusMessage::error(std::string msg) {
        m_ErrorMsg = msg;
        m_Ok = false;
    }

    bool StatusMessage::isOk(std::string& error) const {
        if (!m_Ok) {
            error = m_ErrorMsg;
        }
        return m_Ok;
    }

    TextureMessage::TextureMessage():m_TextureName(0) {}

    BufferMessage::BufferMessage():m_BufferName(0) {}

    FramebufferMessage::FramebufferMessage():m_BufferEnum(0) {}

    FBOMessage::FBOMessage():m_Name(0) {}

    FBOAttachment::FBOAttachment(uint32_t id):m_Ok(true),m_Id(id) {}

    void FBOAttachment::error(std::string msg) {
        m_Ok = false;
        m_ErrorMsg = msg;
    }

    bool FBOAttachment::isOk(std::string& msg) const {
        msg = m_ErrorMsg;
        return m_Ok;
    }

    ShaderMessage::ShaderMessage():m_Name(0) {}

    SetBreakPointsMessage::SetBreakPointsMessage(const std::set<Entrypoint>& breakpoints):m_BreakPoints(breakpoints) {}

    std::set<Entrypoint> SetBreakPointsMessage::get() const {
        return m_BreakPoints;
    }

};