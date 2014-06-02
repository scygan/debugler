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

#ifndef GL_OBJECT_NAMESPACE_H
#define GL_OBJECT_NAMESPACE_H

#include "gl-objects.h"

#include <DGLNet/protocol/ctxobjname.h>

#include <memory>
#include <mutex>
#include <type_traits>

namespace dglState {

template<class ObjType>
class GLObjectNS {
public:

    GLObjectNS() {
        clear();
    }

    ObjType* getObject(GLuint name) {
        if (name < kFastLookupSize) {
            return m_ObjectsFastLookup[name];
        }

        typename std::map<GLuint, ObjType>::iterator i  = m_Objects.find(name);

        if (i != m_Objects.end()) {
            return &(*i).second;
        }
        return nullptr;
    }

    template <class CreateParams>
    typename std::enable_if<!std::is_same<CreateParams, void>::value, ObjType*>::type
    getOrCreateObject(GLuint name, CreateParams createParams) {
        
        ObjType* ret = getObject(name);

        if (!ret) {
            typename std::map<GLuint, ObjType>::iterator i = 
                m_Objects.insert(std::pair<GLuint, ObjType>(
                    name, ObjType(name, createParams))
                ).first;

            ret = &(*i).second;

            if (name < kFastLookupSize) {
                m_ObjectsFastLookup[name] = ret;
            }
        }
        return ret;
    }

    template <class CreateParams>
    typename std::enable_if<std::is_same<CreateParams, void>::value, ObjType*>::type
        getOrCreateObject(GLuint name) {

            ObjType* ret = getObject(name);

            if (!ret) {
                typename std::map<GLuint, ObjType>::iterator i = 
                    m_Objects.insert(std::pair<GLuint, ObjType>(
                    name, ObjType(name))
                    ).first;

                ret = &(*i).second;

                if (name < kFastLookupSize) {
                    m_ObjectsFastLookup[name] = ret;
                }
            }
            return ret;
    }

    void deleteObject(GLuint name) {

        if (name < kFastLookupSize) {
            m_ObjectsFastLookup[name] = nullptr;
        }

        typename std::map<GLuint, ObjType>::iterator i = m_Objects.find(name);
        if (i != m_Objects.end()) {
            m_Objects.erase(i);
        }
    }

    std::set<dglnet::ContextObjectName> getReport(opaque_id_t ctxName) {
        std::set<dglnet::ContextObjectName> ret;

        for (typename std::map<GLuint, ObjType>::iterator i = m_Objects.begin();
            i != m_Objects.end(); i++) {
                ret.insert(
                    dglnet::ContextObjectName(ctxName, i->second.getName(), i->second.getTarget()));
        }

        return ret;
    }

    void clear() {
        for (size_t i = 0; i < kFastLookupSize; i++) {
            m_ObjectsFastLookup[i] = nullptr;
        }
        m_Objects.clear();
    }

private:
    static const int kFastLookupSize = 100;

    std::map<GLuint, ObjType> m_Objects;
    ObjType* m_ObjectsFastLookup[kFastLookupSize]; 
};

class GLShareableObjectNS {
public:
    GLObjectNS<GLTextureObj>      m_Textures;
    GLObjectNS<GLBufferObj>       m_Buffers;
};

class GLObjectNameSpaces;

class GLShareableObjectsAccessor {
public:
    GLShareableObjectsAccessor(GLObjectNameSpaces& namespaces);
inline GLShareableObjectNS& get() { return *m_Namespaces; }

private:
    std::shared_ptr<GLShareableObjectNS> m_Namespaces;
    std::lock_guard<std::recursive_mutex> m_lock;
};

class GLObjectNameSpaces {
public:

    GLObjectNameSpaces(); 

    GLShareableObjectsAccessor getShared();

    void clear();

    GLObjectNS<GLProgramObj>      m_Programs;
    GLObjectNS<GLShaderObj>       m_Shaders;
    GLObjectNS<GLFBObj>           m_FBOs;
    GLObjectNS<GLRenderbufferObj> m_Renderbuffers;
private:
    
    std::shared_ptr<GLShareableObjectNS> m_Shared;
    std::recursive_mutex m_SharedMutex;

    friend GLShareableObjectsAccessor;
    
};

}// namespace dglState

#endif