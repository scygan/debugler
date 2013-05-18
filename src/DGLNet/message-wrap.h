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