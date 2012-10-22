#ifndef _MESSAGE_H
#define _MESSAGE_H

//for boost serialization
#pragma warning(disable:4244 4308)

#include "DGLCommon/gl-types.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp> 

namespace dglnet {

class Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {}

public:
    virtual ~Message() {}
};

class CurrentCallStateMessage: public Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<Message>(*this);
        ar & m_entrp;
    }

public:
    CurrentCallStateMessage(Entrypoint entrp):m_entrp(entrp) {}
    CurrentCallStateMessage() {}

private:
    Entrypoint m_entrp;
};

};


#endif