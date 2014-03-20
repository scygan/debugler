/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#ifndef _MSGUTILS_H
#define _MSGUTILS_H

#include <DGLNet/protocol/fwd.h>
#include <DGLNet/protocol/ctxobjname.h>

#include <set>
#include <vector>

namespace dglnet {
    namespace message {
        namespace utils {

            class ContextReport {
               public:
                template <class Archive>
                void serialize(Archive& ar, const unsigned int) {
                    ar& m_Id;
                    ar& m_TextureSpace;
                    ar& m_BufferSpace;
                    ar& m_ShaderSpace;
                    ar& m_ProgramSpace;
                    ar& m_FBOSpace;
                    ar& m_FramebufferSpace;
                    ar& m_TextureUnitSpace;
                }

                ContextReport() : m_Id(0) {}
                ContextReport(opaque_id_t id) : m_Id(id) {}
                opaque_id_t m_Id;
                std::set<ContextObjectName> m_TextureSpace;
                std::set<ContextObjectName> m_BufferSpace;
                std::set<ContextObjectName> m_ShaderSpace;
                std::set<ContextObjectName> m_ProgramSpace;
                std::set<ContextObjectName> m_FBOSpace;
                std::set<ContextObjectName> m_FramebufferSpace;
                std::vector<std::set<ContextObjectName> > m_TextureUnitSpace;
            };



            class ReplyBase {
            public:
                template <class Archive>
                void serialize(Archive& /*ar*/, const unsigned int) {}

                virtual ~ReplyBase() {}
            };

        }

        enum class ObjectType {
            Texture,
            FBO,
            Framebuffer,
            Shader,
            Program,
            Buffer,
            GPU,
            State,
            Invalid
        };


        enum class StepMode {
            CALL,
            DRAW_CALL,
            FRAME
        };

    }
}

#endif
