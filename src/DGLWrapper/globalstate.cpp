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

#include "globalstate.h"

#include "action-manager.h"
#include "api-loader.h"
#include "debugger.h"

struct GlobalStateImpl {

    ActionManager m_ActionManager;
    APILoader m_ApiLoader;

    DGLDebugController m_DebugController;
    DGLConfiguration   m_Configuration;
};

std::unique_ptr<GlobalStateImpl> GlobalState::s_GlobImpl;


GlobalStateImpl* GlobalState::GetImpl() {
    if (!s_GlobImpl) {
        s_GlobImpl = std::unique_ptr<GlobalStateImpl>(new GlobalStateImpl());
    }
    return s_GlobImpl.get();
}

void GlobalState::reset() {
    if (s_GlobImpl) {
        s_GlobImpl.reset();
    }
}

ActionManager& GlobalState::getActionManager() {
    return GetImpl()->m_ActionManager;
}

DGLDebugController& GlobalState::getDebugController() {
    return GetImpl()->m_DebugController;
}

DGLConfiguration& GlobalState::getConfiguration() {
    return GetImpl()->m_Configuration;
}
 


struct EarlyGlobalStateImpl {
    APILoader m_ApiLoader;
};

std::unique_ptr<EarlyGlobalStateImpl> EarlyGlobalState::s_GlobImpl;


EarlyGlobalStateImpl* EarlyGlobalState::GetImpl() {
    if (!s_GlobImpl) {
        s_GlobImpl = std::unique_ptr<EarlyGlobalStateImpl>(new EarlyGlobalStateImpl());
    }
    return s_GlobImpl.get();
}

void EarlyGlobalState::reset() {
    if (s_GlobImpl) {
        s_GlobImpl.reset();
    }
}


APILoader& EarlyGlobalState::getApiLoader() {
    return GetImpl()->m_ApiLoader;
}