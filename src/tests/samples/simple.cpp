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

#include <sstream>
#include <vector>

namespace gl {

    class NamedObject {
    public: 
        NamedObject():m_name(0) {}
        GLuint Name() {
            return m_name;
        }
    protected:
        GLuint m_name;
    };


    class Shader: public NamedObject {
    public:
        Shader(GLenum stage, std::string source) {
            m_name = glCreateShader(stage);

            const GLchar* strings[] = { source.c_str() };

            glShaderSource(m_name, 1, strings, NULL);
        }

        void Compile() {
            glCompileShader(m_name);

            GLint m_CompileStatus; 
            glGetShaderiv(m_name, GL_COMPILE_STATUS, &m_CompileStatus);
            if (m_CompileStatus != GL_TRUE) {
                std::ostringstream error; 
                error << "SHADER COMPILE FAILED: " << std::endl;

                GLsizei infoLogLength; 
                glGetShaderiv(m_name, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                glGetShaderInfoLog(m_name, infoLog.size(), &infoLogLength, &infoLog[0]);
                infoLog[infoLog.size() - 1] = '\0';
                error << &infoLog[0];

                throw std::runtime_error(error.str());
            }
        }

        ~Shader() {
            glDeleteShader(m_name);
        };
    };

    typedef std::shared_ptr<Shader> ShaderPtr;

    ShaderPtr CreateShader(GLenum stage, const std::string source, bool compile = true) {
        ShaderPtr ret = std::make_shared<Shader>(stage, source);
        if (compile) {
            ret->Compile();
        }
        return ret;
    }


    class Program: public NamedObject {
    public:
        Program() {
            m_name = glCreateProgram();
        }
        ~Program() {
            glDeleteShader(m_name);
        };

        void Attach(ShaderPtr shader) {
            glAttachShader(m_name, shader->Name());
        }

        void Link() {
            glLinkProgram(m_name);
            GLint m_LinkStatus; 
            glGetProgramiv(m_name, GL_LINK_STATUS, &m_LinkStatus);
            if (m_LinkStatus != GL_TRUE) {
                std::ostringstream error; 
                error << "SHADER LINK FAILED: " << std::endl;

                GLsizei infoLogLength;
                glGetProgramiv(m_name, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                glGetProgramInfoLog(m_name, infoLog.size(), &infoLogLength, &infoLog[0]);
                infoLog[infoLog.size() - 1] = '\0';
                error << &infoLog[0];

                throw std::runtime_error(error.str());
            }
        }
    };

    typedef std::shared_ptr<Program> ProgramPtr;

    ProgramPtr CreateProgram(const std::string vsh, const std::string fsh, bool link = true) {
        gl::ShaderPtr vShader = CreateShader(GL_VERTEX_SHADER, vsh);
        gl::ShaderPtr fShader = CreateShader(GL_FRAGMENT_SHADER, fsh);

        gl::ProgramPtr ret = std::make_shared<Program>();

        ret->Attach(vShader);
        ret->Attach(fShader);

        if (link) {
            ret->Link();
        }
        return ret;
    }
}


class SampleSimple: public Sample {
    
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
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4(0.4, 0.5, 0.8, 1.0);\n"
                "}\n";

            m_program = gl::CreateProgram(vshSrc, fshSrc);
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

std::shared_ptr<Sample> Sample::Create_Simple() {
    return std::make_shared<SampleSimple>();
}