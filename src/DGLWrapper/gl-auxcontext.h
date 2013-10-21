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


#ifndef GL_AUX_CONTEXT_H
#define GL_AUX_CONTEXT_H

#include <DGLCommon/gl-types.h>
#include <map>
#include <memory>

class DGLDisplayState;

namespace dglState {

    class GLContext; 

    class GLAuxContext;

    class GLAuxContextSession {
    public:
        ~GLAuxContextSession();
        GLAuxContextSession(GLAuxContext*);
    private:
        GLAuxContext* m_ctx;
    };

    
    class GLAuxContextSurface {
    public:
        GLAuxContextSurface(const DGLDisplayState* display, opaque_id_t pixfmt);
        ~GLAuxContextSurface();
        opaque_id_t getId() const;
    private:
        opaque_id_t m_DisplayId;
        opaque_id_t m_Id;
    };


    class GLAuxContext {
    public:
        GLAuxContext(const GLContext*);
        ~GLAuxContext();

        GLAuxContextSession makeCurrent();

        class GLQueries {
        public:
            GLQueries(GLAuxContext*);

            void setupInitialState();

            void auxGetTexImage(GLuint name, GLenum target, GLint level, GLenum format, GLenum type,  int width, int height, GLvoid* pixels);

        private:
            GLuint getTextureShaderProgram(GLenum target, GLenum format);

            GLuint fbo, vao, vbo, rbo, vshobj;
            std::map<std::string, GLuint> programs;

            bool m_InitialState;

            GLAuxContext* m_AuxCtx;

        } queries;

    private:

        opaque_id_t choosePixelFormat(opaque_id_t preferred, opaque_id_t displayId);

        void doRefCurrent();
        void doUnrefCurrent();
        
        opaque_id_t m_Id, m_PixelFormat;
        const GLContext* m_Parrent;
        int m_MakeCurrentRef;

        std::shared_ptr<GLAuxContextSurface> m_AuxSurface;

        friend class GLAuxContextSession;
    };

} //namespace
#endif
