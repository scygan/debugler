#ifndef _MESSAGE_H
#define _MESSAGE_H

//for boost serialization
#pragma warning(disable:4244 4308)

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp> 

#include <DGLCommon/gl-data.h>

#include "transport.h"

namespace dglnet {

class MessageBase {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {}

public:
    virtual void handle(MessageHandler* h) const  = 0;
    
    virtual ~MessageBase() {}
};



template<class Pod>
class Message: public MessageBase, public Pod {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<MessageBase>(*this);
        save(ar);
    }
    virtual void handle(MessageHandler* h) const { h->doHandle(*static_cast<const Pod*>(this)); }

public:
    Message(){}
    Message(const Pod& p):Pod(p) {};
};

};


#endif