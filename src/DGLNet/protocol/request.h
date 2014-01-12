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

#include <DGLNet/protocol/ctxobjname.h>
#include <DGLNet/protocol/msgutils.h>

#ifndef REQUEST_H
#define REQUEST_H

namespace dglnet {

class DGLRequest {
   public:
    template <class Archive>
    void serialize(Archive& /*ar*/, const unsigned int) {}
    virtual ~DGLRequest() {}
};

namespace request {

class QueryResource : public DGLRequest {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<DGLRequest>(*this);
        ar& m_Type;
        ar& m_ObjectName;
    }

    QueryResource() : m_Type(message::ObjectType::Invalid) {}
    QueryResource(message::ObjectType type, ContextObjectName name)
            : m_Type(type), m_ObjectName(name) {}
    message::ObjectType m_Type;
    ContextObjectName m_ObjectName;
};

class EditShaderSource : public DGLRequest {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<DGLRequest>(*this);
        ar& m_Context;
        ar& m_ShaderId;
        ar& m_Reset;
        if (!m_Reset) {
            ar& m_Source;
        }
    }

    EditShaderSource() {}
    EditShaderSource(opaque_id_t context, gl_t shaderId, bool reset,
                     std::string source = "");

    opaque_id_t m_Context;
    gl_t m_ShaderId;
    bool m_Reset;
    std::string m_Source;
};

class ForceLinkProgram : public DGLRequest {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<DGLRequest>(*this);
        ar& m_Context;
        ar& m_ProgramId;
    }

    ForceLinkProgram() {}
    ForceLinkProgram(opaque_id_t context, gl_t programId);

    opaque_id_t m_Context;
    gl_t m_ProgramId;
};

}    // namespace request
}    // namespace dglnet

#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::request::QueryResource,     drQR)
REGISTER_CLASS(dglnet::request::EditShaderSource,  drESS)
REGISTER_CLASS(dglnet::request::ForceLinkProgram,  drFLP)
#endif

#endif    // REQUEST_H
