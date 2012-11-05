#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <DGLCommon/gl-types.h>
#include <DGLCommon/gl-serialized.h>

#include <DGLNet/serializer-fwd.h>
#include <set>

namespace dglnet {

class BreakedCallMessage;
class ContinueBreakMessage;
class QueryCallTraceMessage;
class CallTraceMessage;


class MessageHandler {
public:
    virtual void doHandle(const BreakedCallMessage&);
    virtual void doHandle(const ContinueBreakMessage&);
    virtual void doHandle(const QueryCallTraceMessage&);
    virtual void doHandle(const CallTraceMessage&);
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
        ar & m_ProgramSpace;
    }
public:
    ContextReport() {}
    ContextReport(int32_t id) {}
    int32_t m_Id;
    std::set<uint32_t> m_TextureSpace;
    std::set<uint32_t> m_BufferSpace;
    std::set<uint32_t> m_ProgramSpace;
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
        ar & m_JustOneStep;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
   ContinueBreakMessage(){}
   ContinueBreakMessage(bool breaked, bool justOneStep = false):m_Breaked(breaked),m_JustOneStep(justOneStep) {}
   bool isBreaked() const;
   bool isJustOneStep() const;

private:
    bool m_Breaked;
    bool m_JustOneStep;
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

};


#endif