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

#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <DGLCommon/gl-types.h>

class DGLPixelTransfer;

namespace dglState {
class GLContext;
}

namespace glutils {

class MSAADownSampler {
   public:
    MSAADownSampler(dglState::GLContext* context, GLenum attTarget, GLenum att,
                    GLuint fboName, GLenum attInternalFormat,
                    DGLPixelTransfer* transfer, int width, int height);
    ~MSAADownSampler();

    GLuint getDownsampledFBO();

   private:
    dglState::GLContext* m_Context;
    GLuint m_DownSampledFBO;
    GLuint m_DownsampledResourceTarget;
    GLuint m_DownsampledResource;
    GLuint m_Texture;
    GLuint m_FBO;
};

GLenum textTargetToBindableTarget(GLenum);

/**
 * Get texture currently bound to given target
 */
GLuint getBoundTexture(GLenum target);

}    // namespace glutils
#endif