#ifndef _MESSAGE_H
#define _MESSAGE_H

//for boost serialization
#pragma warning(disable:4244)

#include "DGLCommon/gl-types.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace dglnet {

class Message {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {}
};


class CurrentCallStateMessage: public Message {
    friend class boost::serialization::access;
public:
    CurrentCallStateMessage(Entrypoint entrp):m_entrp(entrp) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

private:
    Entrypoint m_entrp;
};

};

#endif