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

#include "action-manager.h"



// Action Manager ctor.
// Called on library init.
//
// Setups action dispatch table
ActionManager::ActionManager() {
    
    // set default action for all entrypoints (std debugging routines)
    std::shared_ptr<actions::ActionBase> defAction(new actions::DefaultAction());
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        RegisterAction(i, defAction);
    }

    //other actions are self-registrable (so they can choose only a subset of entrypoints)

    //GL error tracing
    actions::GLGetErrorAction::Register(*this);

    //GL get proc address tracing and handling
    actions::GetProcAddressAction::Register(*this);

#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
    // tracing of surfaces.
    //
    // Needed due to EGL_CONFIG_ID queried broken on one implementation.
    actions::SurfaceAction::Register(*this);
#endif

    //Context tracing
    actions::ContextAction::Register(*this);

    //Debug context enforcements
    actions::DebugContextAction::Register(*this);

    //object tracing
    actions::TextureAction::Register(*this);
    actions::TextureFormatAction::Register(*this);
    actions::BufferAction::Register(*this);
    actions::FBOAction::Register(*this);
    actions::ProgramAction::Register(*this);
    actions::ShaderAction::Register(*this);

    //debug output functionality tracing
    actions::DebugOutputCallback::Register(*this);
}

void ActionManager::RegisterAction(Entrypoint entryp, std::shared_ptr<actions::ActionBase> action) {

    //Add old action as a child to new action
    action->SetPrev(actions[entryp]);

    //swap the new and old action
    std::swap(actions[entryp], action);
}

// Global action manager object
ActionManager g_ActionManager;
