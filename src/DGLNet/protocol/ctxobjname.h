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


#ifndef CTXOBJNAME_H
#define CTXOBJNAME_H

#include <DGLCommon/gl-types.h>

namespace dglnet {

class ContextObjectName {
public:
    ContextObjectName();
    ContextObjectName(opaque_id_t context, gl_t name, gl_t target = 0);
    virtual ~ContextObjectName();
    template<class Archive>
    void serialize(Archive & ar, const unsigned int) {
        ar & m_Name;
        ar & m_Context;
        ar & m_Target;
    }

    virtual bool operator==(const ContextObjectName&rhs) const;

    virtual bool operator<(const ContextObjectName&rhs) const;

    gl_t m_Name;
    opaque_id_t m_Context;
    gl_t m_Target;
};

} //namespace dglnet

#endif //REQUEST_H
