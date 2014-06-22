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

#include "sample.h"
#include "glutil.h"

#include <vector>

#ifndef OPENGL_ES2

class SampleTexture2DMSAA : public Sample {

    virtual void startup() override {
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        GLfloat vertexPositions[] = {0.5,  0.5, 0.0, 1.0, 0.5,  -0.5, 0.0, 1.0,
            -0.5, 0.5, 0.0, 1.0, -0.5, -0.5, 0.0, 1.0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClearDepthf(1.0);

        const char* vshSrc =
            "#version 140\n"
            "attribute vec4 position;\n"
            "out vec2 texPos;\n"
            "\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "    texPos = position.xy * 0.5 + 0.5;\n"
            "}\n";

        const char* fshSrc =
            "#version 140\n"
            "    uniform sampler2DMS sampler0;\n"
            "in vec2 texPos;\n"
            "out vec4 oColor;\n"
            "void main()\n"
            "{\n"
            "    oColor  = texelFetch(sampler0, ivec2(texPos * 16 + vec2(0.5)), 0);\n"
            "}\n";


        m_program = gl::CreateProgram(vshSrc, fshSrc);
        glUseProgram(m_program->Name());

        glGenTextures(1, &m_tex);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 16, 32, 0);

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex, 0);
    }

    virtual void render() override {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(float(102)/255.f, float(127)/255.f, float(204)/255.f, float(255)/255.f);
        glClear(GL_COLOR_BUFFER_BIT);


        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glGetError();
    }

    virtual void shutdown() override {
        glDeleteBuffers(1, &m_vbo);
        glDeleteTextures(1, &m_tex);
        glDeleteFramebuffers(1, &m_fbo);
    }

private:
    GLuint m_vbo;
    GLuint m_tex;
    GLuint m_fbo;
    gl::ProgramPtr m_program;
};

REGISTER_SAMPLE(SampleTexture2DMSAA, "texture2d_msaa");

#endif