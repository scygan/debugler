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

void DGLThreadState::resetAPI() {
    privAPI.m_Current = NULL;
    privAPI.m_EGLApi = EGL_OPENGL_ES_API;
}

void DGLThreadState::releaseAPI() { get()->resetAPI(); }

DGLThreadState* DGLThreadState::get() {

    static THREAD_LOCAL DGLThreadState ret;

    static THREAD_LOCAL bool s_Initialized = false;
    if (!s_Initialized) {
        ret.resetAPI();
        ret.privDebugger.m_ActionProcessing = false;
        s_Initialized = true;
    }

    return &ret;
}

dglState::GLContext* DGLThreadState::getCurrentCtx() {
    return privAPI.m_Current;
}

void DGLThreadState::bindContext(DGLDisplayState* dpy, opaque_id_t ctxId,
                                 dglState::NativeSurfaceBase* drawSurface,
                                 dglState::NativeSurfaceBase* readSurface) {

    dglState::GLContext* currentCtx = gc;
    dglState::GLContext* newCtx = NULL;

    if (currentCtx && currentCtx->getId() == ctxId) {
        newCtx = currentCtx;
    } else if (ctxId) {
        newCtx =
            &(*(dpy->getContext(dglState::GLContextVersion::Type::UNSUPPORTED,
                                ctxId)->second));
    }

    if (currentCtx != newCtx) {
        if (newCtx) {
            privAPI.m_Current = newCtx;
            newCtx->bound();
        } else {
            privAPI.m_Current = NULL;
        }

        if (currentCtx) {
            if (currentCtx->unboundMayDelete()) {
                dpy->deleteContext(currentCtx->getId());
            }
        }
    }

    if (gc) {
        gc->setNativeSurfaces(readSurface, drawSurface);
    }
}

void DGLThreadState::bindEGLApi(EGLenum api) { privAPI.m_EGLApi = api; }

EGLenum DGLThreadState::getEGLApi() { return privAPI.m_EGLApi; }

bool DGLThreadState::enterActionProcessing() {
    if (privDebugger.m_ActionProcessing) {
        return false;
    } else {
        privDebugger.m_ActionProcessing = true;
        return true;
    }
}

bool DGLThreadState::inActionProcessing() {
    return privDebugger.m_ActionProcessing;
}

void DGLThreadState::leaveActionProcessing() {
    privDebugger.m_ActionProcessing = false;
}
