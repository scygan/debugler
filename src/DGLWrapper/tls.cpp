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

#include "tls.h"
#include "gl-state.h"
#include "display.h"

DGLThreadState::DGLThreadState():m_Current(NULL), m_EGLApi(EGL_OPENGL_ES_API) {}

void DGLThreadState::release() {
    s_CurrentThreadState.reset(NULL);
}

DGLThreadState* DGLThreadState::get() {
    DGLThreadState* ret = s_CurrentThreadState.get();
    if (!ret) {
        s_CurrentThreadState.reset(ret = new DGLThreadState());
    }
    return ret;
}

boost::thread_specific_ptr<DGLThreadState> DGLThreadState::s_CurrentThreadState;

dglState::GLContext* DGLThreadState::getCurrentCtx() {
    return m_Current;
}

void DGLThreadState::bindContext(DGLDisplayState* dpy, opaque_id_t ctxId, dglState::NativeSurfaceBase* readSurface) {

    dglState::GLContext* currentCtx = gc;
    dglState::GLContext* newCtx = NULL;

    if (currentCtx && currentCtx->getId() == ctxId) {
        newCtx = currentCtx;
    } else if (ctxId) {
        newCtx = &(*(dpy->ensureContext(dglState::GLContextVersion::UNSUPPORTED, ctxId)->second));
    }

    if (currentCtx != newCtx) {
        if (newCtx) {
            m_Current = newCtx;
            newCtx->bound();
        } else {
            m_Current = NULL;
        }

        if (currentCtx) {
            if (currentCtx->unboundMayDelete()) {
                dpy->deleteContext(currentCtx->getId());
            }
        }
    }

    if (gc) {
        gc->setNativeReadSurface(readSurface);
    }    
}

void DGLThreadState::bindEGLApi(EGLenum api) {
    m_EGLApi = api;
}

EGLenum DGLThreadState::getEGLApi() {
    return m_EGLApi;
}