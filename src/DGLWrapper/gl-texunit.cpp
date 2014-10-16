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

#include "gl-texunit.h"
#include "pointers.h"

namespace dglState {


    
    void TextureUnit::bindTexture(GLenum target, GLuint name) {
        if (name) {
            m_BoundTextures[target] = name;
        } else {
            auto i = m_BoundTextures.find(target);
            if (i != m_BoundTextures.end()) {
                m_BoundTextures.erase(i);
            }
        }
    }

    void TextureUnit::unbindTexture(GLuint name) {
        
        auto i = m_BoundTextures.begin();

        while(i != m_BoundTextures.end()) {
            auto next = i;
            ++next;
            if (i->second == name) {
                bindTexture(i->first, 0);
            }
            i = next;
        }
    }

    std::set<dglnet::ContextObjectName> TextureUnit::report(opaque_id_t ctxId) {
        std::set<dglnet::ContextObjectName> ret;

        for (auto i = m_BoundTextures.begin(); i != m_BoundTextures.end(); ++i) {
            ret.insert(dglnet::ContextObjectName(ctxId, i->second, i->first));
        }

        return ret;
    }

    void AllTextureUnits::init() {
        
        GLint numUnits = 16;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &numUnits);
        
        m_Units.resize(static_cast<size_t>(numUnits));
        
    }

    void AllTextureUnits::bindTexture(GLenum target, GLuint name) {
        
        GLint activeUnit; 
        DIRECT_CALL_CHK(glGetIntegerv)(GL_ACTIVE_TEXTURE, &activeUnit);
        
        activeUnit -= GL_TEXTURE0;

        if (activeUnit >= static_cast<GLint>(m_Units.size())) {
            //that's very strange. 
            m_Units.resize(static_cast<size_t>(activeUnit) + 1);
        }
        m_Units[activeUnit].bindTexture(target, name);

    }

    void AllTextureUnits::unbindTexture(GLuint name) {
        for (size_t i = 0; i < m_Units.size(); i++) {
             m_Units[i].unbindTexture(name);
        }
    }

    std::vector<std::set<dglnet::ContextObjectName> > AllTextureUnits::report(opaque_id_t ctxId) {
        std::vector<std::set<dglnet::ContextObjectName> > ret(m_Units.size());

        for (size_t i = 0; i < m_Units.size(); i++) {
            ret[i] = m_Units[i].report(ctxId);
        }
        return ret;
    }
    
} //dglState