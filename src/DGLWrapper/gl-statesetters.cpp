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

#include "gl-statesetters.h"
#include "gl-context.h"
#include "pointers.h"

namespace dglState {

namespace state_setters {

DefaultPBO::DefaultPBO(GLContext* ctx) : m_Ctx(ctx) {
    if (m_Ctx->hasCapability(GLContext::ContextCap::PixelBufferObjects)) {
        DIRECT_CALL_CHK(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &m_PBO);
    } else {
        m_PBO = 0;
    }
    if (m_PBO) {
        DIRECT_CALL_CHK(glBindBuffer)(GL_PIXEL_PACK_BUFFER, 0);
    }
}
DefaultPBO::~DefaultPBO() {
    if (m_PBO) {
        DIRECT_CALL_CHK(glBindBuffer)(GL_PIXEL_PACK_BUFFER, m_PBO);
    }
}

CurrentFramebuffer::CurrentFramebuffer(GLContext* ctx, GLuint name)
        : m_Ctx(ctx) {
    if (m_Ctx->hasCapability(GLContext::ContextCap::FramebufferObjects)) {
        if (m_Ctx->hasCapability(GLContext::ContextCap::
                                         SeparateReadDrawFramebufferObjects)) {
            // full FBO support
            DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_FRAMEBUFFER_BINDING,
                                           &m_ReadFBO);
            DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_FRAMEBUFFER_BINDING,
                                           &m_DrawFBO);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, name);
        } else {
            // only single draw+read fbo binding is supported
            DIRECT_CALL_CHK(glGetIntegerv)(GL_FRAMEBUFFER_BINDING, &m_DrawFBO);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, name);
        }
    }
}
CurrentFramebuffer::~CurrentFramebuffer() {
    if (m_Ctx->hasCapability(GLContext::ContextCap::FramebufferObjects)) {
        if (m_Ctx->hasCapability(GLContext::ContextCap::
                                         SeparateReadDrawFramebufferObjects)) {
            // full FBO support
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, m_ReadFBO);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, m_DrawFBO);
        } else {
            // only single draw+read fbo binding is supported
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, m_DrawFBO);
        }
    }
}

ReadBuffer::ReadBuffer(GLContext* ctx) : m_Ctx(ctx) {
    if (m_Ctx->hasCapability(GLContext::ContextCap::ReadBufferSelector)) {
        DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &m_ReadBuffer);
    }
}
ReadBuffer::~ReadBuffer() {
    if (m_Ctx->hasCapability(GLContext::ContextCap::ReadBufferSelector)) {
        DIRECT_CALL_CHK(glReadBuffer)(m_ReadBuffer);
    }
}

DrawBuffers::DrawBuffers(GLContext* ctx) : m_Ctx(ctx) {
    if (m_Ctx->hasCapability(GLContext::ContextCap::DrawBuffersMRT)) {
        GLint maxDrawBuffers;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
        m_DrawBuffers.resize(maxDrawBuffers);
        for (GLint i = 0; i < maxDrawBuffers; i++) {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_BUFFER0 + i,
                                           &m_DrawBuffers[i]);
        }
    }
}
DrawBuffers::~DrawBuffers() {
    if (m_Ctx->hasCapability(GLContext::ContextCap::DrawBuffersMRT)) {
        DIRECT_CALL_CHK(glDrawBuffers)(
                static_cast<GLsizei>(m_DrawBuffers.size()),
                reinterpret_cast<GLenum*>(&m_DrawBuffers[0]));
    }
}

RenderBuffer::RenderBuffer() {
    DIRECT_CALL_CHK(glGetIntegerv)(GL_RENDERBUFFER_BINDING, &m_RenderBuffer);
}
RenderBuffer::~RenderBuffer() {
    DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, m_RenderBuffer);
}

PixelStoreAlignment::PixelStoreAlignment(GLContext* ctx) : m_Ctx(ctx) {
    // dump and set pixel store state
    for (int i = 0; i < STATE_SIZE; i++) {
        if (m_Ctx->getVersion().check(GLContextVersion::Type::DT) ||
            (s_StateTable[i].m_ES3 &&
             m_Ctx->getVersion().check(GLContextVersion::Type::ES, 3))) {
            DIRECT_CALL_CHK(glGetIntegerv)(s_StateTable[i].m_Target,
                                           &s_StateTable[i].m_SavedState);
            DIRECT_CALL_CHK(glPixelStorei)(s_StateTable[i].m_Target,
                                           s_StateTable[i].m_State);
        }
    }
}
PixelStoreAlignment::~PixelStoreAlignment() {
    for (int i = 0; i < STATE_SIZE; i++) {
        if (m_Ctx->getVersion().check(GLContextVersion::Type::DT) ||
            (s_StateTable[i].m_ES3 &&
             m_Ctx->getVersion().check(GLContextVersion::Type::ES, 3))) {
            DIRECT_CALL_CHK(glPixelStorei)(s_StateTable[i].m_Target,
                                           s_StateTable[i].m_SavedState);
        }
    }
}

int PixelStoreAlignment::getAligned(int x) {
    int a = s_StateTable[7].m_State;
    return (x + a - 1) & (-a);
}

PixelStoreAlignment::StateEntry PixelStoreAlignment::s_StateTable[STATE_SIZE] =
        {{GL_PACK_SWAP_BYTES, GL_FALSE, false, 0},
         {GL_PACK_LSB_FIRST, GL_FALSE, false, 0},
         {GL_PACK_ROW_LENGTH, 0, true, 0},
         {GL_PACK_IMAGE_HEIGHT, 0, false, 0},
         {GL_PACK_SKIP_ROWS, 0, true, 0},
         {GL_PACK_SKIP_PIXELS, 0, true, 0},
         {GL_PACK_SKIP_IMAGES, 0, false, 0},
         {GL_PACK_ALIGNMENT, 4, true, 0}, };
}
}