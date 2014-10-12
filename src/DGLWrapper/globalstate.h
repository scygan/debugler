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
struct EarlyGlobalStateImpl;

class ActionManager;
class APILoader;
class DGLDebugController;
class DGLConfiguration;
class DynamicLoader;

/* Singleton wrapping all global state in DGLWrapper
*/
class GlobalState {
public:
    GlobalState();
    static ActionManager&      getActionManager();
    static DGLDebugController& getDebugController();
    static DGLConfiguration&   getConfiguration();

    static void reset();

private:
    inline static GlobalStateImpl* GetImpl();

    static std::unique_ptr<GlobalStateImpl> s_GlobImpl;
};

/* Singleton wrapping all global state used from Loader Thread
 * 
 * This is separate from GlobalState, as LoaderThread runs with limited functionality.
 *
*/
class EarlyGlobalState {
public:
    EarlyGlobalState();
    static APILoader&          getApiLoader();
    static DynamicLoader&      getDynLoader();
    static void reset();

private:
    inline static EarlyGlobalStateImpl* GetImpl();

    static std::unique_ptr<EarlyGlobalStateImpl> s_GlobImpl;

    static int s_DummySymbol;
};


#endif

