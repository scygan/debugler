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

namespace dglnet {

ContextObjectName::ContextObjectName():m_Name(0), m_Context(0), m_Target(0) {}
ContextObjectName::ContextObjectName(opaque_id_t context, gl_t name, gl_t target):m_Name(name),m_Context(context),m_Target(target) {}
ContextObjectName::~ContextObjectName() {}

bool ContextObjectName::operator==(const ContextObjectName&rhs) const {

    //it is crucial that m_Target is not get into account here (ID + ctxID is enough to indentify an object and m_Target is optional)

    return m_Context == rhs.m_Context && m_Name == rhs.m_Name;
}

bool ContextObjectName::operator<(const ContextObjectName&rhs) const {

    //it is crucial that m_Target is not get into account here (ID + ctxID is enough to indentify an object and m_Target is optional)

    if (m_Context < rhs.m_Context)
        return true;
    if (m_Context > rhs.m_Context)
        return false;
    if (m_Name < rhs.m_Name)
        return true;
    return false;
}

} //namespace dglnet
