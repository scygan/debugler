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

#include <DGLCommon/gl-types.h>
#include <DGLNet/protocol/entrypoint.h>
#include <DGLNet/protocol/dglconfiguration.h>
#include <DGLNet/protocol/ctxobjname.h>

#include <DGLNet/serializer-fwd.h>
#include <set>
#include <boost/shared_ptr.hpp>

namespace dglnet {

namespace message {
    //generic messages
    class Hello;
    class Configuration;
    class BreakedCall;
    class ContinueBreak;
    class QueryCallTrace;
    class CallTrace;

    class Request;
    class RequestReply;

    class SetBreakPoints;
}


class MessageHandler {
public:
    virtual void doHandle(const message::Hello&);
    virtual void doHandle(const message::Configuration&);
    virtual void doHandle(const message::BreakedCall&);
    virtual void doHandle(const message::ContinueBreak&);
    virtual void doHandle(const message::QueryCallTrace&);
    virtual void doHandle(const message::CallTrace&);

    virtual void doHandle(const message::Request&);
    virtual void doHandle(const message::RequestReply&);

    virtual void doHandle(const message::SetBreakPoints&);

    virtual void doHandleDisconnect(const std::string& why) = 0;
    virtual ~MessageHandler() {}
private:
    void unsupported();
};

class Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& /*ar*/, const unsigned int) {}

public:
    virtual void handle(MessageHandler*) const = 0;

    virtual ~Message() {}
};


namespace message {

class Hello: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_ProcessName;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    Hello() {}
    Hello(std::string name):m_ProcessName(name) {}
    std::string m_ProcessName;
};

class Configuration: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_config.m_BreakOnGLError;
        ar & m_config.m_BreakOnDebugOutput;
        ar & m_config.m_BreakOnCompilerError;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    Configuration() {}
    Configuration(const DGLConfiguration& conf):m_config(conf) {}

    DGLConfiguration m_config;
};

class BreakedCall: public Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_entryp;
        ar & m_TraceSize;
        ar & m_CtxReports;
        ar & m_CurrentCtx;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:

    class ContextReport {
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & m_Id;
            ar & m_TextureSpace;
            ar & m_BufferSpace;
            ar & m_ShaderSpace;
            ar & m_ProgramSpace;
            ar & m_FBOSpace;
            ar & m_FramebufferSpace;
        }
    public:
        ContextReport() {}
        ContextReport(opaque_id_t id):m_Id(id) {}
        opaque_id_t m_Id;
        std::set<ContextObjectName> m_TextureSpace;
        std::set<ContextObjectName> m_BufferSpace;
        std::set<ContextObjectName> m_ShaderSpace;
        std::set<ContextObjectName> m_ProgramSpace;
        std::set<ContextObjectName> m_FBOSpace;
        std::set<ContextObjectName> m_FramebufferSpace;
    };


    BreakedCall(CalledEntryPoint entryp, value_t traceSize, opaque_id_t currentCtx, std::vector<ContextReport> ctxReports):m_entryp(entryp), m_TraceSize(traceSize), m_CtxReports(ctxReports), m_CurrentCtx(currentCtx) {}
    BreakedCall() {}

    CalledEntryPoint m_entryp;
    value_t m_TraceSize;
    std::vector<ContextReport> m_CtxReports;
    opaque_id_t m_CurrentCtx;
};

class ContinueBreak: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Breaked;
        ar & m_InStepMode;
        ar & m_StepMode;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:

   enum StepMode {
       STEP_CALL,
       STEP_DRAW_CALL,
       STEP_FRAME
   };

   ContinueBreak(){}
   ContinueBreak(StepMode stepMode):m_Breaked(false),m_InStepMode(true), m_StepMode(stepMode) {}
   ContinueBreak(bool breaked):m_Breaked(breaked),m_InStepMode(false) {}
   bool isBreaked() const;
   std::pair<bool, StepMode> getStep() const;

private:
    bool m_Breaked;
    bool m_InStepMode;
    StepMode m_StepMode;
};

class QueryCallTrace: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_StartOffset;
        ar & m_EndOffset;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryCallTrace(){}
    QueryCallTrace(value_t startOffset, value_t endOffset):m_StartOffset(startOffset), m_EndOffset(endOffset) {}

    value_t m_StartOffset;
    value_t m_EndOffset;
};

class CallTrace: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_StartOffset;
        ar & m_Trace;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    CallTrace(){}
    CallTrace(const std::vector<CalledEntryPoint>& trace, int start):m_StartOffset(start), m_Trace(trace) {}

    value_t m_StartOffset;
    std::vector<CalledEntryPoint> m_Trace;
};

}// namespace message

class DGLRequest;

namespace message {


class Request: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_RequestId;
        ar & m_Request;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

    value_t m_RequestId;
    static value_t s_RequestId;

public:
    Request();
    Request(DGLRequest*);
    boost::shared_ptr<DGLRequest> m_Request;
    int getId() const;
};

class RequestReply: public Message {
public:
    friend class boost::serialization::access;

    RequestReply();

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Ok;
        ar & m_ErrorMsg;
        ar & m_RequestId;
        ar & m_Reply;
    }
    void error(std::string msg);
    bool isOk(std::string& error) const;

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

    class ReplyBase {
        public:
            template<class Archive>
            void serialize(Archive& /*ar*/, const unsigned int) {}
        
            virtual ~ReplyBase() {}
    };

    int getId() const;

    value_t m_RequestId;
    boost::shared_ptr<ReplyBase> m_Reply;
private:
    bool m_Ok;
    std::string m_ErrorMsg;
};

class SetBreakPoints: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_BreakPoints;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    SetBreakPoints() {}
    SetBreakPoints(const std::set<Entrypoint>&);
    std::set<Entrypoint> get() const;

private:
    std::set<Entrypoint> m_BreakPoints;

};

} //namespace message
} //namespace dglnet


#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::message::Hello)
REGISTER_CLASS(dglnet::message::Configuration)
REGISTER_CLASS(dglnet::message::BreakedCall)
REGISTER_CLASS(dglnet::message::ContinueBreak)
REGISTER_CLASS(dglnet::message::QueryCallTrace)
REGISTER_CLASS(dglnet::message::CallTrace)
REGISTER_CLASS(dglnet::message::Request)
REGISTER_CLASS(dglnet::message::RequestReply)
REGISTER_CLASS(dglnet::message::RequestReply::ReplyBase)
REGISTER_CLASS(dglnet::message::SetBreakPoints)
#endif

#endif
