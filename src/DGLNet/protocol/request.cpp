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

#include "request.h"

namespace dglnet {
namespace request {

EditShaderSource::EditShaderSource(opaque_id_t context, gl_t shaderId,
                                   bool reset, std::string source)
        : m_Context(context),
          m_ShaderId(shaderId),
          m_Reset(reset),
          m_Source(source) {}

ForceLinkProgram::ForceLinkProgram(opaque_id_t context, gl_t programId)
        : m_Context(context), m_ProgramId(programId) {}

}    // namespace resource
}    // namespace dglnet
