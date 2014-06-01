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

#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <memory>


struct GlobalStateImpl;

class ActionManager;
class APILoader;
class DGLDebugController;
class DGLConfiguration;

/* Singleton wrapping all global state in DGLWrapper
*/
class GlobalState {
public:
    GlobalState();
    static ActionManager&      getActionManager();
    static APILoader&          getApiLoader();
    static DGLDebugController& getDebugController();
    static DGLConfiguration&   getConfiguration();

    static void reset();

private:
    inline static GlobalStateImpl* GetImpl();

    static std::unique_ptr<GlobalStateImpl> s_GlobImpl;
};

#endif

