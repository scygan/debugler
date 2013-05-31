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

#include <DGLNet/protocol/message.h>
#include <DGLNet/protocol/resource.h>
#include <DGLNet/protocol/ctxobjname.h>

#ifndef REQUEST_H
#define REQUEST_H

namespace dglnet {

class DGLRequest {
public:
    template<class Archive>
    void serialize(Archive& /*ar*/, const unsigned int) {}
    virtual ~DGLRequest() {}
};

namespace request {

class QueryResource: public DGLRequest {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<DGLRequest>(*this);
        ar & m_Type;
        ar & m_ObjectName;
    }
public:
    QueryResource() {}
    QueryResource(DGLResource::ObjectType type, ContextObjectName name):m_Type(type), m_ObjectName(name) {}
    DGLResource::ObjectType m_Type;
    ContextObjectName m_ObjectName;
};

class EditShaderSource: public DGLRequest {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & boost::serialization::base_object<DGLRequest>(*this);
        ar & m_Context;
        ar & m_ShaderId;
        ar & m_Reset;
        if (!m_Reset) {
            ar & m_Source;
        }
    }

public:
    EditShaderSource() {}
    EditShaderSource(opaque_id_t context, gl_t shaderId, bool reset, std::string source = "");

    opaque_id_t m_Context;
    gl_t m_ShaderId;
    bool m_Reset;
    std::string m_Source;
};

} //namespace request
} //namespace dglnet

#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::request::QueryResource)
REGISTER_CLASS(dglnet::request::EditShaderSource)
#endif

#endif //REQUEST_H
