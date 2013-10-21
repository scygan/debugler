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

#ifndef GL_STATESETTERS_H
#define GL_STATESETTERS_H

#include <DGLCommon/gl-headers.h>
#include <vector>

namespace dglState {

    class GLContext;

namespace state_setters {

    class DefaultPBO {
    public:
        DefaultPBO(GLContext* ctx);
        ~DefaultPBO();
    private:
		GLContext* m_Ctx;
        GLint m_PBO;
    };

    class CurrentFramebuffer {
    public:
        CurrentFramebuffer(GLContext* ctx, GLuint name);
        ~CurrentFramebuffer();
    private:
		GLContext* m_Ctx;
        GLint m_ReadFBO, m_DrawFBO;
    };

    class ReadBuffer {
    public:
        ReadBuffer(GLContext* ctx);
        ~ReadBuffer();
    private:
		GLContext* m_Ctx;
        GLint m_ReadBuffer;
    };

    class DrawBuffers {
    public:
        DrawBuffers(GLContext* ctx);
        ~DrawBuffers();
    private:
        GLContext* m_Ctx;
        std::vector<GLint> m_DrawBuffers;        
    };

    class RenderBuffer {
    public:
        RenderBuffer();
        ~RenderBuffer();
    private:
        GLint m_RenderBuffer;
    };

    class PixelStoreAlignment {
#define STATE_SIZE 8
    public:
        PixelStoreAlignment(GLContext* ctx);
        ~PixelStoreAlignment();
    public:
        int getAligned(int x);
    private:

        static struct StateEntry {
            GLenum m_Target;
            GLint m_State;
            bool m_ES3;
            GLint m_SavedState;
        } s_StateTable[STATE_SIZE];
        GLContext* m_Ctx;
    };
}
}

#endif //GL_STATESETTERS_H
