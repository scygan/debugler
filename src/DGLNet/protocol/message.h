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
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <DGLNet/protocol/fwd.h>
#include <DGLCommon/gl-types.h>
#include <DGLNet/protocol/entrypoint.h>
#include <DGLNet/protocol/dglconfiguration.h>
#include <DGLNet/protocol/ctxobjname.h>
#include <DGLNet/protocol/msgutils.h>

#include <set>
#include <boost/shared_ptr.hpp>

namespace dglnet {

class Message {
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& /*ar*/, const unsigned int) {}

   public:
    virtual void handle(MessageHandler*) const = 0;

    virtual ~Message() {}
};

namespace message {


class Hello : public Message {
public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_ProcessName;
    }

    Hello() {}
    Hello(std::string name) : m_ProcessName(name) {}
    std::string m_ProcessName;

private:
    virtual void handle(MessageHandler* h) const;
};

class Configuration : public Message {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_config.m_BreakOnGLError;
        ar& m_config.m_BreakOnDebugOutput;
        ar& m_config.m_BreakOnCompilerError;
        ar& m_config.m_ForceDebugContext;
        ar& m_config.m_ForceDebugContextES;
    }

    Configuration() {}
    Configuration(const DGLConfiguration& conf) : m_config(conf) {}

    DGLConfiguration m_config;
   private:
    virtual void handle(MessageHandler* h) const;
};

class BreakedCall : public Message {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_entryp;
        ar& m_TraceSize;
        ar& m_CtxReports;
        ar& m_CurrentCtx;
    }

    BreakedCall(CalledEntryPoint entryp, value_t traceSize,
                opaque_id_t currentCtx, std::vector<utils::ContextReport> ctxReports)
            : m_entryp(entryp),
              m_TraceSize(traceSize),
              m_CtxReports(ctxReports),
              m_CurrentCtx(currentCtx) {}
    BreakedCall()
            : m_entryp(NO_ENTRYPOINT, 0), m_TraceSize(0), m_CurrentCtx(0) {}

    CalledEntryPoint m_entryp;
    value_t m_TraceSize;
    std::vector<utils::ContextReport> m_CtxReports;
    opaque_id_t m_CurrentCtx;
   private:
    virtual void handle(MessageHandler* h) const;
};

class ContinueBreak : public Message {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_Breaked;
        ar& m_InStepMode;
        ar& m_StepMode;
    }

   public:
    ContinueBreak()
            : m_Breaked(false),
              m_InStepMode(false),
              m_StepMode(StepMode::CALL) {}
    ContinueBreak(StepMode stepMode)
            : m_Breaked(false), m_InStepMode(true), m_StepMode(stepMode) {}
    ContinueBreak(bool breaked)
            : m_Breaked(breaked),
              m_InStepMode(false),
              m_StepMode(StepMode::CALL) {}
    bool isBreaked() const;
    std::pair<bool, StepMode> getStep() const;

   private:
    virtual void handle(MessageHandler* h) const;

    bool m_Breaked;
    bool m_InStepMode;
    StepMode m_StepMode;
};

class Terminate : public Message {
   public:

    template <class Archive>
    void serialize(Archive& ar, const unsigned) {
        ar& boost::serialization::base_object<Message>(*this);
    }

   private:
    virtual void handle(MessageHandler* h) const;
};

class QueryCallTrace : public Message {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_StartOffset;
        ar& m_EndOffset;
    }

    QueryCallTrace() : m_StartOffset(0), m_EndOffset(0) {}
    QueryCallTrace(value_t startOffset, value_t endOffset)
            : m_StartOffset(startOffset), m_EndOffset(endOffset) {}

    value_t m_StartOffset;
    value_t m_EndOffset;
   private:
    virtual void handle(MessageHandler* h) const;
};

class CallTrace : public Message {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_StartOffset;
        ar& m_Trace;
    }

    CallTrace() : m_StartOffset(0) {}
    CallTrace(const std::vector<CalledEntryPoint>& trace, int start)
            : m_StartOffset(start), m_Trace(trace) {}

    value_t m_StartOffset;
    std::vector<CalledEntryPoint> m_Trace;
private:
    virtual void handle(MessageHandler* h) const;
};

}    // namespace message

class DGLRequest;

namespace message {

class Request : public Message {
    value_t m_RequestId;
    static value_t s_RequestId;

   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_RequestId;
        ar& m_Request;
    }

    Request();
    Request(DGLRequest*);
    boost::shared_ptr<DGLRequest> m_Request;
    int getId() const;
private:
    virtual void handle(MessageHandler* h) const;
};

class RequestReply : public Message {
   public:


    RequestReply();

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_Ok;
        ar& m_ErrorMsg;
        ar& m_RequestId;
        ar& m_Reply;
    }
    void error(std::string msg);
    bool isOk(std::string& error) const;
    int getId() const;

    value_t m_RequestId;
    boost::shared_ptr<utils::ReplyBase> m_Reply;

   private:
    virtual void handle(MessageHandler* h) const;
    bool m_Ok;
    std::string m_ErrorMsg;
};

class SetBreakPoints : public Message {
   public:

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Message>(*this);
        ar& m_BreakPoints;
    }

    SetBreakPoints() {}
    SetBreakPoints(const std::set<Entrypoint>&);
    std::set<Entrypoint> get() const;

   private:
    virtual void handle(MessageHandler* h) const;
    std::set<Entrypoint> m_BreakPoints;
};

}    // namespace message
}    // namespace dglnet

#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::message::Hello,          dmH)
REGISTER_CLASS(dglnet::message::Configuration,  dmC)
REGISTER_CLASS(dglnet::message::BreakedCall,    dmBC)
REGISTER_CLASS(dglnet::message::ContinueBreak,  dmCB)
REGISTER_CLASS(dglnet::message::QueryCallTrace, dmQCT)
REGISTER_CLASS(dglnet::message::CallTrace,      dmCT)
REGISTER_CLASS(dglnet::message::Terminate,      dmT)
REGISTER_CLASS(dglnet::message::Request,        dmR)
REGISTER_CLASS(dglnet::message::RequestReply,   dmRR)
REGISTER_CLASS(dglnet::message::SetBreakPoints, dmSBP)
#endif

#endif
