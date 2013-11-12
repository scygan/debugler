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

#ifdef OPENGL_ES2
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include <memory>
#include <string>

namespace gl {

class NamedObject {
   public:
    NamedObject();
    GLuint Name();

   protected:
    GLuint m_name;
};

class Shader : public NamedObject {
   public:
    Shader(GLenum, std::string);
    void Compile();
    ~Shader();
};

typedef std::shared_ptr<Shader> ShaderPtr;
ShaderPtr CreateShader(GLenum stage, const std::string source,
                       bool compile = true);

class Program : public NamedObject {
   public:
    Program();
    ~Program();

    void Attach(ShaderPtr shader);
    void Link();
};
typedef std::shared_ptr<Program> ProgramPtr;
ProgramPtr CreateProgram(const std::string vsh, const std::string fsh,
                         bool link = true);
}
