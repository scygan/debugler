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

#include "gl-utils.h"
#include "gl-context.h"
#include "pointers.h"
#include "api-loader.h"

#include <DGLNet/protocol/pixeltransfer.h>
#include <DGLCommon/def.h>

#include <stdexcept>

namespace glutils {

MSAADownSampler::MSAADownSampler(dglState::GLContext* context, GLenum attTarget,
                                 GLenum att, GLuint fboName,
                                 GLenum attInternalFormat,
                                 DGLPixelTransfer* transfer, int width,
                                 int height)
        : m_Context(context),
          m_DownSampledFBO(0),
          m_DownsampledResourceTarget(attTarget),
          m_DownsampledResource(0),
          m_FBO(fboName) {
    // this is a multisample render target. We must downsample it before reading

    if (m_DownsampledResourceTarget == GL_RENDERBUFFER) {

        // create renderbuffer for downsampling
        DIRECT_CALL_CHK(glGenRenderbuffers)(1, &m_DownsampledResource);

        DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER,
                                            m_DownsampledResource);
        DIRECT_CALL_CHK(glRenderbufferStorage)(
                GL_RENDERBUFFER, attInternalFormat, width, height);
        DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, 0);

    } else if (m_DownsampledResourceTarget == GL_TEXTURE_2D_MULTISAMPLE) {

        DIRECT_CALL_CHK(glGenTextures)(1, &m_DownsampledResource);

        GLuint lastTexture;
        if (!getBoundTexture(GL_TEXTURE_2D, lastTexture)) {
            //should not happen for 2D textures
            DGL_ASSERT(0);
        }

        DIRECT_CALL_CHK(glBindTexture)(GL_TEXTURE_2D, m_DownsampledResource);
        DIRECT_CALL_CHK(glTexImage2D)(GL_TEXTURE_2D, 0, attInternalFormat,
                                      width, height, 0,
                                      (GLenum)transfer->getFormat(),
                                      (GLenum)transfer->getType(), NULL);

        DIRECT_CALL_CHK(glBindTexture)(GL_TEXTURE_2D, lastTexture);

    } else if (m_DownsampledResourceTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {

        DIRECT_CALL_CHK(glGenTextures)(1, &m_DownsampledResource);

        GLuint lastTexture;
        if (!getBoundTexture(GL_TEXTURE_2D_ARRAY, lastTexture)) {
            //should not happen for 2D_ARRAY texture
            DGL_ASSERT(0);
        }

        DIRECT_CALL_CHK(glBindTexture)(GL_TEXTURE_2D_ARRAY,
                                       m_DownsampledResource);
        // depth is 1 here. we need only one slice of array to downsample to
        DIRECT_CALL_CHK(glTexImage3D)(GL_TEXTURE_2D_ARRAY, 0, attInternalFormat,
                                      width, height, 1, 0,
                                      (GLenum)transfer->getFormat(),
                                      (GLenum)transfer->getType(), NULL);

        DIRECT_CALL_CHK(glBindTexture)(GL_TEXTURE_2D_ARRAY, lastTexture);
    } else {
        throw std::runtime_error("Unsupported multisample texture target: " +
                                 GetGLEnumName(attTarget, GLEnumGroup::TextureTarget));
    }

    // create framebuffer used for downsampling, set it as draw
    DIRECT_CALL_CHK(glGenFramebuffers)(1, &m_DownSampledFBO);
    DIRECT_CALL_CHK(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, m_DownSampledFBO);

    if (m_DownsampledResourceTarget == GL_RENDERBUFFER) {
        DIRECT_CALL_CHK(glFramebufferRenderbuffer)(GL_DRAW_FRAMEBUFFER, att,
                                                   GL_RENDERBUFFER,
                                                   m_DownsampledResource);
    } else if (m_DownsampledResourceTarget == GL_TEXTURE_2D_MULTISAMPLE) {
        DIRECT_CALL_CHK(glFramebufferTexture2D)(GL_DRAW_FRAMEBUFFER, att,
                                                GL_TEXTURE_2D,
                                                m_DownsampledResource, 0);
    } else if (m_DownsampledResourceTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
        DIRECT_CALL_CHK(glFramebufferTextureLayer)(GL_DRAW_FRAMEBUFFER, att,
                                                   m_DownsampledResource, 0, 0);
    } else {
        DGL_ASSERT(0);
    }

    GLint blitMask = 0;

    if (att != GL_DEPTH_ATTACHMENT && att != GL_STENCIL_ATTACHMENT &&
        att != GL_DEPTH_STENCIL_ATTACHMENT) {
        // select buffers for downsampling
        if (m_Context->hasCapability(
                    dglState::GLContext::ContextCap::ReadBuffer))
            DIRECT_CALL_CHK(glReadBuffer)(att);
        if (m_Context->hasCapability(
                    dglState::GLContext::ContextCap::DrawBuffersMRT))
            DIRECT_CALL_CHK(glDrawBuffer)(att);

        blitMask |= GL_COLOR_BUFFER_BIT;
    } else {
        switch (att) {
            case GL_DEPTH_ATTACHMENT:
                blitMask |= GL_DEPTH_BUFFER_BIT;
                break;
            case GL_STENCIL_ATTACHMENT:
                blitMask |= GL_STENCIL_BUFFER_BIT;
                break;
            case GL_DEPTH_STENCIL_ATTACHMENT:
                blitMask |= GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
                break;
        }
    }

    // downsample
    DIRECT_CALL_CHK(glBlitFramebuffer)(0, 0, width, height, 0, 0, width, height,
                                       blitMask, GL_NEAREST);
}

MSAADownSampler::~MSAADownSampler() {
    DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, m_FBO);
    if (m_DownSampledFBO) {
        DIRECT_CALL_CHK(glDeleteFramebuffers)(1, &m_DownSampledFBO);
    }
    if (m_DownsampledResource) {
        if (m_DownsampledResourceTarget == GL_RENDERBUFFER) {
            DIRECT_CALL_CHK(glDeleteRenderbuffers)(1, &m_DownsampledResource);
        } else {
            DIRECT_CALL_CHK(glDeleteTextures)(1, &m_DownsampledResource);
        }
    }
}

GLuint MSAADownSampler::getDownsampledFBO() { return m_DownSampledFBO; }

GLenum textTargetToBindableTarget(GLenum target) {
    switch (target) {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return GL_TEXTURE_CUBE_MAP;
        default:
            return target;
    }
}

bool getBoundTexture(GLenum target, GLuint& name) {
#if DGL_HAVE_WA(ARM_MALI_EMU_GETTERS_OVERFLOW)
    // WA for buggy ARM Mali OpenGL ES 3.0 emulator, where to really much data
    // is returned from glGetIntegerv
    // It was measured that bug is causing to overwrite 192*4 bytes of memory.
    GLint lastTexture[192];
#else
    GLint lastTexture[1];
#endif

    switch (target) {
        case GL_TEXTURE_1D:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_1D, lastTexture);
            break;
        case GL_TEXTURE_2D:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_2D, lastTexture);
            break;
        case GL_TEXTURE_2D_MULTISAMPLE:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_2D_MULTISAMPLE,
                                           lastTexture);
            break;
        case GL_TEXTURE_RECTANGLE:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_RECTANGLE,
                                           lastTexture);
            break;
        case GL_TEXTURE_1D_ARRAY:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_1D_ARRAY,
                                           lastTexture);
            break;
        case GL_TEXTURE_CUBE_MAP:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_CUBE_MAP,
                                           lastTexture);
            break;
        case GL_TEXTURE_3D:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_3D, lastTexture);
            break;
        case GL_TEXTURE_2D_ARRAY:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_2D_ARRAY,
                                           lastTexture);
            break;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            DIRECT_CALL_CHK(glGetIntegerv)(
                    GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, lastTexture);
            break;
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_CUBE_MAP_ARRAY,
                                           lastTexture);
            break;
        case GL_TEXTURE_EXTERNAL_OES:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_EXTERNAL_OES,
                                           lastTexture);
        default:
            DGL_ASSERT(0);
            return false;
    }

    name =  lastTexture[0];
    return true;
}

}    // namespace glutils