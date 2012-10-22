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
class DebugStepMessage;

class MessageHandler {
public:
    virtual void doHandle(const BreakedCallMessage&);
    virtual void doHandle(const DebugStepMessage&);
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


class DebugStepMessage: public Message {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
    }

    virtual void handle(MessageHandler* h) const { h->doHandle(*this); }

public:
   DebugStepMessage(){}
};

};


#endif