#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <DGLCommon/gl-types.h>
#include <DGLCommon/gl-serialized.h>
#include <DGLCommon/dglconfiguration.h>

#include <DGLNet/serializer-fwd.h>
#include <set>

namespace dglnet {

class ConfigurationMessage;
class BreakedCallMessage;
class ContinueBreakMessage;
class QueryCallTraceMessage;
class CallTraceMessage;
class QueryTextureMessage;
class TextureMessage;
class QueryBufferMessage;
class BufferMessage;
class QueryFramebufferMessage;
class FramebufferMessage;
class QueryFBOMessage;
class FBOMessage;
class QueryShaderMessage;
class ShaderMessage;
class QueryProgramMessage;
class ProgramMessage;
class SetBreakPointsMessage;


class MessageHandler {
public:
    virtual void doHandle(const ConfigurationMessage&);
    virtual void doHandle(const BreakedCallMessage&);
    virtual void doHandle(const ContinueBreakMessage&);
    virtual void doHandle(const QueryCallTraceMessage&);
    virtual void doHandle(const CallTraceMessage&);
    virtual void doHandle(const QueryTextureMessage&);
    virtual void doHandle(const TextureMessage&);
    virtual void doHandle(const QueryBufferMessage&);
    virtual void doHandle(const BufferMessage&);
    virtual void doHandle(const QueryFramebufferMessage&);
    virtual void doHandle(const FramebufferMessage&);
    virtual void doHandle(const QueryFBOMessage&);
    virtual void doHandle(const FBOMessage&);
    virtual void doHandle(const QueryShaderMessage&);
    virtual void doHandle(const ShaderMessage&);
    virtual void doHandle(const QueryProgramMessage&);
    virtual void doHandle(const ProgramMessage&);
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

class ContextObjectName {
public:
    ContextObjectName() {}
    ContextObjectName(uint32_t name):m_Name(name) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_Name;
    }

    bool operator<(const ContextObjectName&rhs) const {
        return m_Name < rhs.m_Name;
    }

    uint32_t m_Name;
};

class ContextObjectNameTarget: public ContextObjectName {
public:
    ContextObjectNameTarget() {}
    ContextObjectNameTarget(uint32_t name, uint32_t target = 0):ContextObjectName(name), m_Target(target) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<ContextObjectName>(*this);
        ar & m_Target;
    }

    uint32_t m_Target;
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
    ContextReport(int32_t id):m_Id(id) {}
    int32_t m_Id;
    std::set<ContextObjectName> m_TextureSpace;
    std::set<ContextObjectName> m_BufferSpace;
    std::set<ContextObjectNameTarget> m_ShaderSpace;
    std::set<ContextObjectName> m_ProgramSpace;
    std::set<ContextObjectName> m_FBOSpace;
    std::set<ContextObjectName> m_FramebufferSpace;
};

class ConfigurationMessage: public Message, public DGLConfiguration {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_BreakOnGLError;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    ConfigurationMessage() {}
    ConfigurationMessage(const DGLConfiguration& conf):DGLConfiguration(conf) {}
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
    BreakedCallMessage(CalledEntryPoint entryp, uint32_t traceSize, uint32_t currentCtx, std::vector<ContextReport> ctxReports):m_entryp(entryp), m_TraceSize(traceSize), m_CurrentCtx(currentCtx), m_CtxReports(ctxReports) {}
    BreakedCallMessage() {}

    CalledEntryPoint m_entryp;
    uint32_t m_TraceSize;
    std::vector<ContextReport> m_CtxReports;
    uint32_t m_CurrentCtx;
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
    QueryCallTraceMessage(int32_t startOffset, int32_t endOffset):m_StartOffset(startOffset), m_EndOffset(endOffset) {}

    uint32_t m_StartOffset;
    uint32_t m_EndOffset;
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

    uint32_t m_StartOffset;
    std::vector<CalledEntryPoint> m_Trace;
};

class QueryTextureMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_TextureName;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryTextureMessage(){}
    QueryTextureMessage(int32_t name):m_TextureName(name) {}

    uint32_t m_TextureName;
};

class TextureLevel {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_Width;
        ar & m_Height;
        ar & m_Channels;
        ar & m_Pixels;
    }
public:
    int32_t m_Width, m_Height, m_Channels;
    std::vector<int8_t> m_Pixels;
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

class TextureMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_TextureName;
        ar & m_Levels;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    TextureMessage();

    uint32_t m_TextureName;
    std::vector<TextureLevel> m_Levels;
};

class QueryBufferMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_BufferName;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryBufferMessage(){}
    QueryBufferMessage(int32_t name):m_BufferName(name) {}

    uint32_t m_BufferName;
};


class BufferMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_BufferName;
        ar & m_Data;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    BufferMessage();

    uint32_t m_BufferName;
    std::vector<char> m_Data;
};

class QueryFramebufferMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_BufferEnum;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryFramebufferMessage(){}
    QueryFramebufferMessage(int32_t bufferEnum):m_BufferEnum(bufferEnum) {}

    uint32_t m_BufferEnum;
};

class FramebufferMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_BufferEnum;
        ar & m_Width;
        ar & m_Height;
        ar & m_Channels;
        ar & m_Pixels;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    FramebufferMessage();

    uint32_t m_BufferEnum;
    int32_t m_Width, m_Height, m_Channels;
    std::vector<int8_t> m_Pixels;
};

class QueryFBOMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Name;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryFBOMessage(){}
    QueryFBOMessage(int32_t m_Name):m_Name(m_Name) {}

    uint32_t m_Name;
};

class FBOAttachment {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_Ok;
        ar & m_ErrorMsg;
        ar & m_Width;
        ar & m_Height;
        ar & m_Channels;
        ar & m_Pixels;
        ar & m_Id;
    }
public:
    FBOAttachment() {}
    FBOAttachment(uint32_t id);

    void error(std::string msg);
    bool isOk(std::string& error) const;


    int32_t m_Width, m_Height, m_Channels;
    std::vector<int8_t> m_Pixels;
    uint32_t m_Id;
private:
    bool m_Ok;
    std::string m_ErrorMsg;
};

class FBOMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_Name;
        ar & m_Attachments;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    FBOMessage();

    uint32_t m_Name;
    std::vector<FBOAttachment> m_Attachments;
};

class QueryShaderMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Name;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryShaderMessage(){}
    QueryShaderMessage(int32_t m_Name):m_Name(m_Name) {}

    uint32_t m_Name;
};

class ShaderMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_Name;
        ar & m_Sources;
        ar & m_CompileStatus;
        ar & m_Target;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    ShaderMessage();

    uint32_t m_Name, m_Target;
    std::vector<std::string> m_Sources;
    std::pair<std::string, uint32_t> m_CompileStatus;
};

class QueryProgramMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_Name;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    QueryProgramMessage(){}
    QueryProgramMessage(int32_t m_Name):m_Name(m_Name) {}

    uint32_t m_Name;
};

class ProgramMessage: public StatusMessage {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<StatusMessage>(*this);
        ar & m_Name;
        ar & mLinkStatus;
        ar & m_AttachedShaders;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    ProgramMessage(){}
    ProgramMessage(int32_t m_Name):m_Name(m_Name) {}

    uint32_t m_Name;
    std::pair<std::string, uint32_t> mLinkStatus;
    std::vector<std::pair<uint32_t, uint32_t>> m_AttachedShaders;
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