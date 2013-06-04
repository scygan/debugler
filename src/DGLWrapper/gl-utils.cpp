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
#include "pointers.h"
#include "api-loader.h"

#include <cassert>

namespace glutils {

MSAADownSampler::MSAADownSampler(GLenum attType, /*GLenum attTarget,*/ GLenum att, GLuint fboName, GLenum attInternalFormat, int width, int height):m_DownSampledFBO(0), m_DownsampledResourceType(attType), m_DownsampledResource(0), m_FBO(fboName) {
    //this is a multisample render target. We must downsample it before reading

    if (m_DownsampledResourceType == GL_RENDERBUFFER) {
        //create renderbuffer for downsampling
        DIRECT_CALL_CHK(glGenRenderbuffers)(1, &m_DownsampledResource);
        DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, m_DownsampledResource);
        DIRECT_CALL_CHK(glRenderbufferStorage)(GL_RENDERBUFFER, attInternalFormat, width, height);
        DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, 0);
    } else if (m_DownsampledResourceType == GL_TEXTURE) {
        DIRECT_CALL_CHK(glGenTextures)(1, &m_DownsampledResource);
        assert(!"not implemented");
 //       glBindTexture(attTarget, m_DownsampledResource);
    } else { assert(0); }

    //create framebuffer used for downsampling, set it as draw
    DIRECT_CALL_CHK(glGenFramebuffers)(1, &m_DownSampledFBO);
    DIRECT_CALL_CHK(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, m_DownSampledFBO);
    DIRECT_CALL_CHK(glFramebufferRenderbuffer)(GL_DRAW_FRAMEBUFFER, att, m_DownsampledResourceType, m_DownsampledResource);
    

    GLint blitMask = 0;

    if (att != GL_DEPTH_ATTACHMENT && att != GL_STENCIL_ATTACHMENT && att != GL_DEPTH_STENCIL_ATTACHMENT) {
        //select buffers for downsampling
        DIRECT_CALL_CHK(glReadBuffer)(att); 
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

    //downsample
    DIRECT_CALL_CHK(glBlitFramebuffer)(0, 0, width, height, 0, 0, width, height, blitMask, GL_NEAREST);
}

MSAADownSampler::~MSAADownSampler() {
    DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, m_FBO);
    if (m_DownSampledFBO) {
        DIRECT_CALL_CHK(glDeleteFramebuffers)(1, &m_DownSampledFBO);
    }
    if (m_DownsampledResource) {
        if (m_DownsampledResourceType == GL_RENDERBUFFER) {
            DIRECT_CALL_CHK(glDeleteRenderbuffers)(1, &m_DownsampledResource);
        } else if (m_DownsampledResourceType == GL_TEXTURE) {
            DIRECT_CALL_CHK(glDeleteTextures)(1, &m_DownsampledResource);
        } else { assert(0); }
        
    }
}

GLuint MSAADownSampler::getDownsampledFBO() {
    return m_DownSampledFBO;
}

} //namespace glutils