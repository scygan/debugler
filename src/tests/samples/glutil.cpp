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


#include "glutil.h"

#include <sstream>
#include <vector>
#include <stdexcept>

namespace gl {

    NamedObject::NamedObject():m_name(0) {}
    GLuint NamedObject::Name() {
        return m_name;
    }


    Shader::Shader(GLenum stage, std::string source) {
        m_name = glCreateShader(stage);

        const GLchar* strings[] = { source.c_str() };

        glShaderSource(m_name, 1, strings, NULL);
    }

    void Shader::Compile() {
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

    Shader::~Shader() {
        glDeleteShader(m_name);
    }

    ShaderPtr CreateShader(GLenum stage, const std::string source, bool compile) {
        ShaderPtr ret = std::make_shared<Shader>(stage, source);
        if (compile) {
            ret->Compile();
        }
        return ret;
    }


   Program::Program() {
        m_name = glCreateProgram();
    }
    Program::~Program() {
        glDeleteShader(m_name);
    }

    void Program::Attach(ShaderPtr shader) {
        glAttachShader(m_name, shader->Name());
    }

    void Program::Link() {
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

    ProgramPtr CreateProgram(const std::string vsh, const std::string fsh, bool link) {
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


