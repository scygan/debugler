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
#include <DGLCommon/gl-serialized.h>
#include <DGLCommon/dglconfiguration.h>

#include <DGLNet/serializer-fwd.h>
#include <set>
#include <boost/shared_ptr.hpp>

namespace dglnet {

class HelloMessage;
class ConfigurationMessage;
class BreakedCallMessage;
class ContinueBreakMessage;
class QueryCallTraceMessage;
class CallTraceMessage;

class QueryResourceMessage;
class ResourceMessage;

class SetBreakPointsMessage;


class MessageHandler {
public:
    virtual void doHandle(const HelloMessage&);
    virtual void doHandle(const ConfigurationMessage&);
    virtual void doHandle(const BreakedCallMessage&);
    virtual void doHandle(const ContinueBreakMessage&);
    virtual void doHandle(const QueryCallTraceMessage&);
    virtual void doHandle(const CallTraceMessage&);

    virtual void doHandle(const QueryResourceMessage&);
    virtual void doHandle(const ResourceMessage&);

    virtual void doHandle(const SetBreakPointsMessage&);
    virtual void doHandleDisconnect(const std::string& why) = 0;
    virtual ~MessageHandler() {}
private:
    void unsupported();
};

class Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {}

public:
    virtual void handle(MessageHandler*) const = 0;

    virtual ~Message() {}
};

class ContextReport {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
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

class HelloMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_ProcessName;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    HelloMessage() {}
    HelloMessage(std::string name):m_ProcessName(name) {}
    std::string m_ProcessName;
};

class ConfigurationMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_config.m_BreakOnGLError;
        ar & m_config.m_BreakOnDebugOutput;
        ar & m_config.m_BreakOnCompilerError;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    ConfigurationMessage() {}
    ConfigurationMessage(const DGLConfiguration& conf):m_config(conf) {}

    DGLConfiguration m_config;
};

class BreakedCallMessage: public Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_entryp;
        ar & m_TraceSize;
        ar & m_CtxReports;
        ar & m_CurrentCtx;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    BreakedCallMessage(CalledEntryPoint entryp, value_t traceSize, opaque_id_t currentCtx, std::vector<ContextReport> ctxReports):m_entryp(entryp), m_TraceSize(traceSize), m_CurrentCtx(currentCtx), m_CtxReports(ctxReports) {}
    BreakedCallMessage() {}

    CalledEntryPoint m_entryp;
    value_t m_TraceSize;
    std::vector<ContextReport> m_CtxReports;
    opaque_id_t m_CurrentCtx;
};

class ContinueBreakMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
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

   ContinueBreakMessage(){}
   ContinueBreakMessage(StepMode stepMode):m_Breaked(false),m_InStepMode(true), m_StepMode(stepMode) {}
   ContinueBreakMessage(bool breaked):m_Breaked(breaked),m_InStepMode(false) {}
   bool isBreaked() const;
   std::pair<bool, StepMode> getStep() const;

private:
    bool m_Breaked;
    bool m_InStepMode;
    StepMode m_StepMode;
};

class QueryCallTraceMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_StartOffset;
        ar & m_EndOffset;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryCallTraceMessage(){}
    QueryCallTraceMessage(value_t startOffset, value_t endOffset):m_StartOffset(startOffset), m_EndOffset(endOffset) {}

    value_t m_StartOffset;
    value_t m_EndOffset;
};

class CallTraceMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_StartOffset;
        ar & m_Trace;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    CallTraceMessage(){}
    CallTraceMessage(const std::vector<CalledEntryPoint>& trace, int start):m_Trace(trace), m_StartOffset(start) {}

    value_t m_StartOffset;
    std::vector<CalledEntryPoint> m_Trace;
};



class StatusMessage: public Message {
public:
    friend class boost::serialization::access;

    StatusMessage();

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Ok;
        ar & m_ErrorMsg;
    }
    void error(std::string msg);
    bool isOk(std::string& error) const;

private:
    bool m_Ok;
    std::string m_ErrorMsg;
};

class QueryResourceMessage: public Message {
    
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_ResourceQueries;
    }
public:

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

    class ResourceQuery {
        
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & m_Type;
            ar & m_ObjectName;
            ar & m_ListenerId;
        }

    public:
        DGLResource::ObjectType m_Type;
        value_t m_ListenerId;
        ContextObjectName m_ObjectName;
    };

    std::vector<ResourceQuery> m_ResourceQueries;
};




class ResourceMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_ListenerId;
        ar & m_Resource;
    }
public:
    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

    value_t m_ListenerId;
    boost::shared_ptr<DGLResource> m_Resource;
};

class SetBreakPointsMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_BreakPoints;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    SetBreakPointsMessage() {}
    SetBreakPointsMessage(const std::set<Entrypoint>&);
    std::set<Entrypoint> get() const;

private:
    std::set<Entrypoint> m_BreakPoints;

};

};


#endif
