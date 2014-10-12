/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include "gl-object-namespace.h"

namespace dglState {

GLShareableObjectsAccessor::GLShareableObjectsAccessor(GLObjectNameSpaces& ns):m_lock(ns.m_SharedMutex), m_Namespaces(ns.m_Shared) {}


GLObjectNameSpaces::GLObjectNameSpaces():m_Shared(std::make_shared<GLShareableObjectNS>()) {}

std::unique_ptr<GLShareableObjectsAccessor> GLObjectNameSpaces::getShared() {
    return std::unique_ptr<GLShareableObjectsAccessor>(new GLShareableObjectsAccessor(*this));
}

void GLObjectNameSpaces::clear() {
    m_Programs.clear();
    m_ProgramPipelines.clear();
    m_Shaders.clear();
    m_FBOs.clear();
    m_Renderbuffers.clear();
}

} //namespace dglState