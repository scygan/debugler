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

//#include <boost/make_shared.hpp>

#include <DGLNet/protocol/msgutils.h>
#include <DGLCommon/wa.h>

#include "display.h"
#include "native-surface.h"
#include "tls.h"

DGLDisplayState::DGLDisplayState(opaque_id_t id, Type type)
        : m_Id(id), m_type(type) {}

opaque_id_t DGLDisplayState::getId() const { return m_Id; }

DGLDisplayState* DGLDisplayState::defDpy(DGLDisplayState::Type type) {
    return get(0, type);
}

DGLDisplayState* DGLDisplayState::get(opaque_id_t dpy,
                                      DGLDisplayState::Type type) {
    std::lock_guard<std::mutex> lock(s_DisplaysMutex);

    std::map<opaque_id_t, std::shared_ptr<DGLDisplayState> >::iterator i =
            s_Displays.find(dpy);

    if (i == s_Displays.end()) {
        i = s_Displays.insert(std::pair<opaque_id_t,
                                        std::shared_ptr<DGLDisplayState> >(
                                      dpy, std::make_shared<DGLDisplayState>(
                                                   dpy, type))).first;
    }
    return i->second.get();
}

void DGLDisplayState::createContext(
        dglState::GLContextVersion version,
        dglState::GLContextCreationData creationData, opaque_id_t id) {
    std::lock_guard<std::mutex> lock(m_ContextListMutex);

    ContextListIter i = m_ContextList.find(id);
    if (i == m_ContextList.end()) {
        m_ContextList.insert(std::pair<opaque_id_t,
                                       std::shared_ptr<dglState::GLContext> >(
                                     id,
                                     std::make_shared<dglState::GLContext>(
                                             this, version, id, creationData)))
                .first;
    }
}

DGLDisplayState::ContextListIter DGLDisplayState::getContext(
        dglState::GLContextVersion version, opaque_id_t id) {
    std::lock_guard<std::mutex> lock(m_ContextListMutex);

    ContextListIter i = m_ContextList.find(id);
    if (i == m_ContextList.end()) {
        i = m_ContextList
                    .insert(std::pair<opaque_id_t,
                                      std::shared_ptr<dglState::GLContext> >(
                             id, std::make_shared<dglState::GLContext>(
                                         this, version, id,
                                         dglState::GLContextCreationData())))
                    .first;
    }
    return i;
}

template <typename NativeSurfaceType>
DGLDisplayState::SurfaceListIter DGLDisplayState::ensureSurface(opaque_id_t id,
                                                                bool lock) {
    if (lock) {
        m_SurfaceListMutex.lock();
    }
    SurfaceListIter i = m_SurfaceList.find(id);
    if (i == m_SurfaceList.end()) {
        i = m_SurfaceList
                    .insert(std::pair<
                             opaque_id_t,
                             std::shared_ptr<dglState::NativeSurfaceBase> >(
                             id,
                             std::make_shared<NativeSurfaceType>(this, id)))
                    .first;
    }
    if (lock) {
        m_SurfaceListMutex.unlock();
    }
    return i;
}
// only WGL + GLX
#ifdef HAVE_LIBRARY_WGL
template DGLDisplayState::SurfaceListIter DGLDisplayState::ensureSurface<
        dglState::NativeSurfaceWGL>(opaque_id_t id, bool lock);
#endif
#ifdef HAVE_LIBRARY_GLX
template DGLDisplayState::SurfaceListIter DGLDisplayState::ensureSurface<
        dglState::NativeSurfaceGLX>(opaque_id_t id, bool lock);
#endif
#if !DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)
template DGLDisplayState::SurfaceListIter DGLDisplayState::ensureSurface<
        dglState::NativeSurfaceEGL>(opaque_id_t id, bool lock);
#endif

DGLDisplayState::SurfaceListIter DGLDisplayState::getSurface(opaque_id_t id) {
    std::lock_guard<std::mutex> guard(m_SurfaceListMutex);
    return m_SurfaceList.find(id);
}

template <typename NativeSurfaceType>
void DGLDisplayState::addSurface(opaque_id_t id, opaque_id_t pixfmt) {
    std::lock_guard<std::mutex> guard(m_SurfaceListMutex);
    m_SurfaceList[id] = std::make_shared<NativeSurfaceType>(this, pixfmt, id);
}
#if DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)
// only EGL (needs pixfmt as query fails on some implementations)
template void DGLDisplayState::addSurface<dglState::NativeSurfaceEGL>(
        opaque_id_t id, opaque_id_t pixfmt);
#endif

void DGLDisplayState::deleteContext(opaque_id_t id) {
    std::lock_guard<std::mutex> guard(m_ContextListMutex);
    if (gc && gc->getId() == id) {
        DGLThreadState::get()->bindContext(this, 0, 0, 0);
    }
    m_ContextList.erase(id);
}

void DGLDisplayState::lazyDeleteContext(opaque_id_t id) {
    dglState::GLContext* ctx =
            &(*(getContext(dglState::GLContextVersion::Type::UNSUPPORTED, id)
                        ->second));
    if (ctx->markForDeletionMayDelete()) {
        deleteContext(id);
    }
}

std::vector<dglnet::message::utils::ContextReport>
DGLDisplayState::describe() {
    std::lock_guard<std::mutex> quard(m_ContextListMutex);

    std::vector<dglnet::message::utils::ContextReport> ret(
            m_ContextList.size());
    size_t j = 0;
    for (ContextListIter i = m_ContextList.begin(); i != m_ContextList.end();
         i++) {
        ret[j++] = i->second->describe();
    }
    return ret;
}

std::vector<dglnet::message::utils::ContextReport>
DGLDisplayState::describeAll() {
    std::vector<dglnet::message::utils::ContextReport> ret;

    std::lock_guard<std::mutex> quard(s_DisplaysMutex);

    for (std::map<opaque_id_t, std::shared_ptr<DGLDisplayState> >::iterator
                 i = s_Displays.begin();
         i != s_Displays.end(); ++i) {

        std::vector<dglnet::message::utils::ContextReport> partialReport =
                i->second->describe();

        std::copy(partialReport.begin(), partialReport.end(),
                  std::back_inserter(ret));
    }
    return ret;
}

DGLDisplayState::Type DGLDisplayState::getType() const { return m_type; }

std::map<opaque_id_t, std::shared_ptr<DGLDisplayState> >
        DGLDisplayState::s_Displays;

std::mutex DGLDisplayState::s_DisplaysMutex;
