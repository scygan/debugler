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
#include <boost/serialization/export.hpp>

#include "request.h"
#include "messagehandler.h"

namespace dglnet {

void MessageHandler::doHandleHello(const message::Hello&) {
    unsupported();
}

void MessageHandler::doHandleConfiguration(const message::Configuration&) {
    unsupported();
}

void MessageHandler::doHandleBreakedCall(const message::BreakedCall&) {
    unsupported();
}

void MessageHandler::doHandleContinueBreak(const message::ContinueBreak&) {
    unsupported();
}

void MessageHandler::doHandleTerminate(const message::Terminate&) {
    unsupported();
}

void MessageHandler::doHandleQueryCallTrace(const message::QueryCallTrace&) {
    unsupported();
}

void MessageHandler::doHandleCallTrace(const message::CallTrace&) {
    unsupported();
}

void MessageHandler::doHandleRequest(const message::Request&) {
    unsupported();
}

void MessageHandler::doHandleRequestReply(const message::RequestReply&) {
    unsupported();
}

void MessageHandler::doHandleSetBreakPoints(const message::SetBreakPoints&) {
    unsupported();
}

void MessageHandler::unsupported() {
    throw std::runtime_error(
            "Message cannot be handled by current message handler object.");
}

namespace message {


#define DEF_MESSAGE_HANDLER(cls) \
    void cls::handle(MessageHandler* h) const { h->doHandle##cls(*this); }

DEF_MESSAGE_HANDLER(Hello)
DEF_MESSAGE_HANDLER(Configuration)
DEF_MESSAGE_HANDLER(BreakedCall)
DEF_MESSAGE_HANDLER(ContinueBreak)
DEF_MESSAGE_HANDLER(Terminate)
DEF_MESSAGE_HANDLER(QueryCallTrace)
DEF_MESSAGE_HANDLER(CallTrace)
DEF_MESSAGE_HANDLER(Request)
DEF_MESSAGE_HANDLER(RequestReply)
DEF_MESSAGE_HANDLER(SetBreakPoints)
#undef DEF_MESSAGE_HANDLER

bool ContinueBreak::isBreaked() const { return m_Breaked; }

std::pair<bool, StepMode> ContinueBreak::getStep() const {
    return std::pair<bool, StepMode>(m_InStepMode, m_StepMode);
}

Request::Request() {
    s_RequestId++;
    m_RequestId = s_RequestId;
}

Request::Request(DGLRequest* request) : m_Request(request) {
    s_RequestId++;
    m_RequestId = s_RequestId;
}

int Request::s_RequestId = 0;

int Request::getId() const { return m_RequestId; }

RequestReply::RequestReply() : m_Ok(true) {}

void RequestReply::error(const std::string& msg) {
    m_ErrorMsg = msg;
    m_Ok = false;
}

bool RequestReply::isOk(std::string& _error) const {
    if (!m_Ok) {
        _error = m_ErrorMsg;
    }
    return m_Ok;
}

int RequestReply::getId() const { return m_RequestId; }

SetBreakPoints::SetBreakPoints(const std::set<Entrypoint>& breakpoints)
        : m_BreakPoints(breakpoints) {}

std::set<Entrypoint> SetBreakPoints::get() const { return m_BreakPoints; }
}
}
