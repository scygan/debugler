#ifndef _MESSAGE_H
#define _MESSAGE_H

//for boost serialization
#pragma warning(disable:4244 4308)

#include "DGLCommon/gl-types.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp> 

namespace dglnet {

class BreakedCallMessage;
class ContinueBreakMessage;

class MessageHandler {
public:
    virtual void doHandle(const BreakedCallMessage&);
    virtual void doHandle(const ContinueBreakMessage&);
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

class BreakedCallMessage: public Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_entrp;
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
    BreakedCallMessage(Entrypoint entrp):m_entrp(entrp) {}
    BreakedCallMessage() {}

    Entrypoint getEntrypoint() const { return m_entrp; }

private:
    Entrypoint m_entrp;
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

};


#endif