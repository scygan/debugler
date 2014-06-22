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

class SampleTexture2DArrayMSAA : public Sample {

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
            "#version 150\n"
            "attribute vec4 position;\n"
            "out vec2 texPos;\n"
            "\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "    texPos = position.xy * 0.5 + 0.5;\n"
            "}\n";

        const char* fshSrc =
            "#version 150\n"
            "uniform sampler2DMSArray sampler0;\n"
            "uniform int layer;\n"
            "in vec2 texPos;\n"
            "out vec4 oColor;\n"
            "void main()\n"
            "{\n"
            "    oColor  = texelFetch(sampler0, ivec3(ivec2(texPos * 16 + vec2(0.5)), layer),  0);\n"
            "}\n";


        m_program = gl::CreateProgram(vshSrc, fshSrc);

        m_layer_uniform_loc = glGetUniformLocation(m_program->Name(), "layer");

        glUseProgram(m_program->Name());

        glGenTextures(1, &m_tex);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_tex);
        glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4, GL_RGBA8, 16, 32, 5, 0);

        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        
    }

    virtual void render() override {

        GLubyte colors[5][4] = {
            {102, 127, 204, 255},
            { 140, 32, 48, 223}, 
            { 74, 189, 232, 239}, 
            { 214, 72, 239, 87},
            { 144, 223, 142, 223 },
        };

        for (int i = 0; i < 5; i++) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_tex, 0, i);
            glClearColor(
                float(colors[i][0])/255.f,
                float(colors[i][1])/255.f,
                float(colors[i][2])/255.f,
                float(colors[i][3])/255.f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 0);

        for (int i = 0; i < 5; i++) {
            glUniform1i(m_layer_uniform_loc, i);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }        
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
    GLint m_layer_uniform_loc;
};

REGISTER_SAMPLE(SampleTexture2DArrayMSAA, "texture2d_array_msaa");

#endif