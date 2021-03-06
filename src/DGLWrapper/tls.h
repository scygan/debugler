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

#ifndef TLS_H
#define TLS_H

#include <DGLCommon/gl-types.h>
#include <DGLCommon/def.h>

class DGLDisplayState;
namespace dglState {
class NativeSurfaceBase;
class GLContext;
}

/**
* Class aggregating all thread-specific state
*/
class DGLThreadState {
   public:
    /**
     * Resets APIs to default state
     */
    void resetAPI();

    /**
     * Setter for current context (should be called just after *MakeCurrent-like
     * calls).
     */
    void bindContext(DGLDisplayState* dpy, opaque_id_t id,
                     dglState::NativeSurfaceBase* drawSurface,
                     dglState::NativeSurfaceBase* readSurface);

    /**
     * Getter for current context (should be called just after *MakeCurrent-like
     * calls).
     */
    inline dglState::GLContext* getCurrentCtx() {
        return privAPI.m_Current;
    }

    /**
     *  Release current TSS - should be called on eglReleaseThread
     */
    static void releaseAPI();

    /**
    *  Get current TSS
    */
    static DGLThreadState* get();

    enum class BoundEGLApi {
        OPENGL_ES_API,
        OPENGL_API,		
        UNKNOWN_API,
    };

    /**
     *  Set current API (only on EGL, called on eglBindApi)
     */
    void bindEGLApi(BoundEGLApi api);

    /**
     *  Set current EGL API
     */
    BoundEGLApi getEGLApi();

    /**
     *  Try enter actions processing
     *  bumps recursion guard
     *  returns true if actions should be processed
     */
    bool enterActionProcessing();

    /**
     *  True if currently in action processing
     */
    bool inActionProcessing();

    /**
     * Leave action processing
     * leaves recursion guard
     */
    void leaveActionProcessing();

    struct PrivAPI {
        /**
         * Thread-space pointer to current context object
         */
        dglState::GLContext* m_Current;

        /**
         * Current EGL api
         */
        BoundEGLApi m_EGLApi;

    } privAPI;

    struct PrivDebuggerState {
        /*
         * Current action recursion guard
         */
        bool m_ActionProcessing;
    } privDebugger;
};

#define gc DGLThreadState::get()->getCurrentCtx()

#endif    // TLS_H
