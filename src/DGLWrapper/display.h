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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <mutex>
#include <map>

#include <DGLCommon/gl-types.h>
#include "gl-state.h"

/**
 * Class aggregating all OpenGL state read needed by debugger
 */
class DGLDisplayState {

    /**
     * Iterator for container of all GL context state objects
     */
    typedef std::map<opaque_id_t,
                     std::shared_ptr<dglState::GLContext> >::iterator
            ContextListIter;

    /**
     * Iterator for container of all native surfaces
     */
    typedef std::map<opaque_id_t,
                     std::shared_ptr<dglState::NativeSurfaceBase> >::iterator
            SurfaceListIter;

   public:
    enum class Type {
        GLX,
        EGL,
        WGL,
    };

    /**
     * Ctor
     */
    DGLDisplayState(opaque_id_t id, Type type);

    /**
     * Native id getter
     */
    opaque_id_t getId() const;

    /**
     * Getter for default display on display-less configurations (like WGL).
     */
    static DGLDisplayState* defDpy(DGLDisplayState::Type type);

    /**
     * Getter for specific display on display-able configurations (like EGL,
     * GLX).
     */
    static DGLDisplayState* get(opaque_id_t dpy, DGLDisplayState::Type type);

    /**
     * Creates new context
     */
    void createContext(dglState::GLContextVersion version,
                       dglState::GLContextCreationData creationData,
                       opaque_id_t id);

    /**
     * Getter for ctx object by given id
     */
    ContextListIter getContext(dglState::GLContextVersion version,
                               opaque_id_t id);

    /**
     * Getter for native surface object by given id (created if not exist). Not
     * usable for EGL
     */
    template <typename NativeSurfaceType>
    SurfaceListIter ensureSurface(opaque_id_t id, bool lock = true);

    /**
     * Getter for native surface object by given id
     */
    SurfaceListIter getSurface(opaque_id_t id);

    /**
     * Add surface by given id and pixelformat. Not usable on WGL
     */
    template <typename NativeSurfaceType>
    void addSurface(opaque_id_t id, opaque_id_t pixfmt);

    /**
     * Method deleting ctx object by given id (should be called when deleted by
     * application)
     */
    void deleteContext(opaque_id_t id);

    /**
     * Method deleting ctx object by given id (should be called when deleted by
     * application). Does not immediately delete when bound
     */
    void lazyDeleteContext(opaque_id_t id);

    /**
     * Getter for short context state report
     */
    std::vector<dglnet::message::utils::ContextReport> describe();

    /**
     * Getter for short context state report from all Displays
     */
    static std::vector<dglnet::message::utils::ContextReport>
            describeAll();

    /**
     * Getter for display type
     */
    Type getType() const;

   private:
    /**
     * Container of all GL context state objects
     */
    std::map<opaque_id_t, std::shared_ptr<dglState::GLContext> >
            m_ContextList;

    /**
     * Container of  all native surfaces
     */
    std::map<opaque_id_t, std::shared_ptr<dglState::NativeSurfaceBase> >
            m_SurfaceList;

    /**
     * Mutex for context container operations
     */
    std::mutex m_ContextListMutex;

    /**
     * Mutex for surface container operations
     */
    std::mutex m_SurfaceListMutex;

    /**
     *  Collection of all displays
     */
    static std::map<opaque_id_t, std::shared_ptr<DGLDisplayState> >
            s_Displays;

    /**
     *  Mutex guarding s_Displays
     */
    static std::mutex s_DisplaysMutex;

    /**
     * Display ID as seen from native API
     */
    opaque_id_t m_Id;

    /**
     * Display type
     */
    Type m_type;
};

#endif    // DISPLAY_H
