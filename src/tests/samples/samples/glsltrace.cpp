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


#include <glsl_optimizer.h>

class SampleGLSLTrace: public Sample {
    
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
                "\n"
                "void main() {\n"
                "    gl_Position = position;\n"
                "}\n";

            const char* fshSrc = 
                "#version 120\n"
                "int bla;\n\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4(0.4, 0.5, 0.8, 1.0); vec2(5);\n"
                "    vec4 bla = vec4(1.0);\n"
                "    bla.x += 0.1f;\n"
                "    int i = 1;\n"
                "    i++;\n"
                "}\n";

            const char* newFsh;

            glslopt_ctx* ctx = glslopt_initialize(kGlslTargetOpenGL);
            glslopt_shader* shader = glslopt_trace (ctx, kGlslOptShaderFragment, fshSrc, 0);
            if (glslopt_get_status (shader)) {
                newFsh = glslopt_get_output (shader);
            } else {
                throw std::runtime_error(glslopt_get_log (shader));
            }

            printf(" ---- \n %s \n\n", newFsh);

            m_program = gl::CreateProgram(vshSrc, fshSrc);
            
            glslopt_shader_delete (shader);
            glslopt_cleanup(ctx);

            glUseProgram(m_program->Name());
    }

    virtual void render() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glGetError();
    }

    virtual void shutdown() override {
        glDeleteBuffers(1, &m_vbo);
    }

private:
    GLuint m_vbo;
    gl::ProgramPtr m_program;
};

REGISTER_SAMPLE(SampleGLSLTrace, "glsltrace");