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

#include <sample.h>

#include <sstream>

class SampleFboMSAA : public Sample {

    virtual void startup() override {
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 256,
                                         256);

        glGenTextures(2, m_textures);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_textures[0]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 256,
                                256, 0);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_textures[1]);
        glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4,
                                GL_DEPTH24_STENCIL8, 256, 256, 256, 0);

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, m_rbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                               GL_TEXTURE_2D_MULTISAMPLE, m_textures[0], 0);
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_textures[1],
                               0, 2);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        glClearDepth(0.4);
    }

    virtual void render() override {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClearColor(0.4f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClearColor(0.8f, 0.5f, 0.4f, 1.0f);
                
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    virtual void shutdown() override {
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteTextures(2, m_textures);
        glDeleteFramebuffers(1, &m_fbo);
    }

   private:
    GLuint m_rbo, m_textures[2], m_fbo;
};

REGISTER_SAMPLE(SampleFboMSAA, "fbo_msaa");