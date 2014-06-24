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

#ifndef DGLGUI_H
#define DGLGUI_H

#ifdef DGLGUI_PCH
// Precompiled headers:
#include "dglqtgui.h"
#include "dglcontroller.h"
#include "dglpixelrectangle.h"
#include "dglmainwindow.h"
#include "dgltabbedview.h"
#endif

#include <DGLCommon/def.h>

#ifdef NDEBUG
#define CONNASSERT(obj1, signal, obj2, slot) connect(obj1, signal, obj2, slot)
#else
#define CONNASSERT(obj1, signal, obj2, slot) \
    DGL_ASSERT(connect(obj1, signal, obj2, slot))

#ifdef HAVE_VLD
#include <vld.h>
#endif

#endif

#define DGL_RES_ICON_TEXTURE_PATH      ":/icons/texture.png"
#define DGL_RES_ICON_BUFFER_PATH       ":/icons/buffer.png"
#define DGL_RES_ICON_FBO_PATH          ":/icons/fbo.png"
#define DGL_RES_ICON_RENDERBUFFER_PATH ":/icons/renderbuffer.png"
#define DGL_RES_ICON_SHADER_PATH       ":/icons/shader.png"
#define DGL_RES_ICON_PROGRAM_PATH      ":/icons/program.png"
#define DGL_RES_ICON_FRAMEBUFFER_PATH  ":/icons/framebuffer.png"
#define DGL_RES_ICON_TEXTUREUNIT_PATH  ":/icons/textureunit.png"
#define DGL_RES_ICON_PPO_PATH          ":/icons/program_pipeline.png"


#endif
