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


class SampleTexture: public Sample {
    
    virtual void startup() override {
            glGenBuffers(1, &m_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            GLfloat vertexPositions[] = {
                 0.5,  0.5,  0.0,  1.0,
                 0.5, -0.5,  0.0,  1.0,
                -0.5, 0.5,  0.0,  1.0,
                -0.5, -0.5,  0.0,  1.0
            };
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClearDepth(1.0);


            const char* vshSrc = 
                "#version 120\n"
                "attribute vec4 position;\n"
                "varying vec2 texPos;\n"
                "\n"
                "void main() {\n"
                "    gl_Position = position;\n"
                "    texPos = position.xy * 0.5 + 0.5;\n"
                "}\n";

            const char* fshSrc = 
                "#version 120\n"
                "uniform sampler2D sampler0;\n"
                "varying vec2 texPos;\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = texture2D(sampler0, texPos);\n"
                "}\n";

            m_program = gl::CreateProgram(vshSrc, fshSrc);
            glUseProgram(m_program->Name());


            glGenTextures(1, &m_tex);
            glBindTexture(GL_TEXTURE_2D, m_tex);


            GLubyte color[] = { 102, 127, 204, 255 };   
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    virtual void render() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glGetError();
    }

    virtual void shutdown() override {
        glDeleteBuffers(1, &m_vbo);
        glDeleteTextures(1, &m_tex);
    }

private:
    GLuint m_vbo;
    GLuint m_tex;
    gl::ProgramPtr m_program;
};


REGISTER_SAMPLE(SampleTexture, "texture");
