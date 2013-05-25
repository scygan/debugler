/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include "message.h"

#include <stdexcept>

namespace dglnet {

    void MessageHandler::doHandle(const HelloMessage&) {
        unsupported();
    }

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

    void MessageHandler::doHandle(const QueryResourceMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const ResourceMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const SetBreakPointsMessage&) {
        unsupported();
    }

    void MessageHandler::doHandle(const EditShaderSourceMessage&) {
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

    SetBreakPointsMessage::SetBreakPointsMessage(const std::set<Entrypoint>& breakpoints):m_BreakPoints(breakpoints) {}

    std::set<Entrypoint> SetBreakPointsMessage::get() const {
        return m_BreakPoints;
    }

    EditShaderSourceMessage::EditShaderSourceMessage(opaque_id_t context, gl_t shaderId, std::string& source):m_Context(context),
        m_ShaderId(shaderId), m_Source(source) {}

};