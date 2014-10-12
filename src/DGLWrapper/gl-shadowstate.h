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

#ifndef GL_SHADOWSTATE_H
#define GL_SHADOWSTATE_H

#include "gl-utils.h"
#include "gl-texunit.h"

namespace dglState {
    
    class GLContextShadowState {
        public: 

            GLContextShadowState();

            /**
             * Getter for texture units container
             */
            inline AllTextureUnits& getTexUnits() { return m_TextureUnits; }


            /**
             * Imemdiate mode setter - must be set, when betweek glBegin()/glEnd(),
             * otherwise spurious GL errors will happen
             * No query will be emitted when in immediate mode
             */
            inline void setImmediateMode(bool immediate) { m_InImmediateMode = immediate; }

            inline bool inImmediateMode() { return m_InImmediateMode; }

            
            GLuint m_CurrentProgram;
        private:
            /**
             * Set to true if betweek glBegin() and glEnd()
             */
            bool m_InImmediateMode;

            /**
             * Shadow of all bound textures
             */
            AllTextureUnits m_TextureUnits;
    };
}

#endif
