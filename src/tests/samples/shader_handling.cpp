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

class SampleShaderHandling: public Sample {
    
    virtual void startup() override {
        const GLchar* source = 
            "//version 120\n"
            "attribute vec4 position;\n"
            "\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";
             
        //simple case: create-delete
        GLuint shader = glCreateShader(GL_VERTEX_SHADER);
        ////test point: we have one shader
        glDeleteShader(shader);
        //test point: shader deleted
        glFlush(); //flush is only to mark case end
        
        //source cache test
        shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader, 1, &source, NULL);
        glDeleteShader(shader);
        //test point: shader deleted, has cached source
        glFlush(); //flush is only to mark case end
            
        //lazy deletion test, create-attach-delete-detach
        shader = glCreateShader(GL_VERTEX_SHADER);
        GLuint program = glCreateProgram();
        glAttachShader(program, shader);
        //test point: program has shader
        glDeleteShader(shader);
        //test point: shader not deleted
        glDetachShader(program, shader);
        //test point: shader deleted, program has no shader
        glDeleteProgram(program);
        glFlush(); //flush is only to mark case end
            
        //lazy deletion test2: create-attach-delete-deleteprogram
        shader = glCreateShader(GL_VERTEX_SHADER);
        program = glCreateProgram();
        glAttachShader(program, shader);
        glDeleteShader(shader);
        //test point: shader not deleted, program has shader
        glDeleteProgram(program);
        //test point: shader deleted
        glFlush(); //flush is only to mark case end
            
        //caching in lazy deletion test (source after delete by detach) 
        shader = glCreateShader(GL_VERTEX_SHADER);
        program = glCreateProgram();
        glAttachShader(program, shader);
        glDeleteShader(shader);
        glShaderSource(shader, 1, &source, NULL);
        glDetachShader(program, shader);
        //test point: cached source
        glDeleteProgram(program);
        glFlush(); //flush is only to mark case end
        
        //caching in lazy deletion test 2 (source after delete by deleteprogram) 
        shader = glCreateShader(GL_VERTEX_SHADER);
        program = glCreateProgram();
        glAttachShader(program, shader);
        glDeleteShader(shader);
        glShaderSource(shader, 1, &source, NULL);
        glDeleteProgram(program);
        //test point: cached source
        glFlush(); //flush is only to mark case end
    }

    virtual void render() override {}

    virtual void shutdown() override {}
};

REGISTER_SAMPLE(SampleShaderHandling, "shader_handling");