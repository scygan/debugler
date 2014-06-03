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

#include <sstream>

class ProgramHandling : public Sample {

    virtual void startup() override {
        const char* vshSrc =
#ifndef OPENGL_ES2
            "#version 120\n"
#endif
            "attribute vec4 position;\n"
            "\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";

        const char* fshSrc =
#ifdef OPENGL_ES2
            "precision mediump float;\n"
#else
            "#version 120\n"
#endif
            "void main()\n"
            "{\n"
            "    gl_FragColor = vec4(0.4, 0.5, 0.8, 1.0);\n"
            "}\n";

        gl::ProgramPtr p = gl::CreateProgram(vshSrc, fshSrc);

        glUseProgram(p->Name());
        p.reset();

        glUseProgram(0);
    }

    virtual void render() override {}

    virtual void shutdown() override {}
};

REGISTER_SAMPLE(ProgramHandling, "program_handling");