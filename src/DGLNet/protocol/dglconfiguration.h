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


#ifndef DGLCONFIGURATION_H
#define DGLCONFIGURATION_H

class DGLConfiguration {
public:
    DGLConfiguration():m_BreakOnGLError(true), m_BreakOnDebugOutput(true), m_BreakOnCompilerError(true) {}
    DGLConfiguration(bool breakOnGLError, bool breakOnDebugOutput, bool breakOnCompilerError):m_BreakOnGLError(breakOnGLError),
        m_BreakOnDebugOutput(breakOnDebugOutput), m_BreakOnCompilerError(breakOnCompilerError) {}
    bool m_BreakOnGLError;
    bool m_BreakOnDebugOutput;
    bool m_BreakOnCompilerError;
};

#endif