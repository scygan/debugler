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

#ifndef GL_TEXUNIT_H
#define GL_TEXUNIT_H

#include <DGLCommon/gl-types.h>

#include <DGLNet/protocol/ctxobjname.h>

#include <vector>
#include <map>
#include <set>

namespace dglState {


    class TextureUnit {
    public:
        void bindTexture(GLenum target, GLuint name);
        void unbindTexture(GLuint name);

        std::set<dglnet::ContextObjectName> report(opaque_id_t ctxId);

    private:
        std::map<GLenum, GLuint> m_BoundTextures;
    };


    class AllTextureUnits {
    public:
        
        void init();        
        void bindTexture(GLenum target, GLuint name);
        void unbindTexture(GLuint name);
        
        std::vector<std::set<dglnet::ContextObjectName> > report(opaque_id_t ctxId);

    private:
        std::vector<TextureUnit> m_Units;
    };

    
    
} //dglState

#endif