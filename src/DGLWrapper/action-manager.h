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

#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <memory>
#include "actions.h"


class ActionManager {
public:
    ActionManager();
    void RegisterAction(Entrypoint entryp, std::shared_ptr<actions::ActionBase> action);

    inline actions::ActionBase& GetAction(Entrypoint entryp) {
        return *actions[entryp];
    }

private:
    std::shared_ptr<actions::ActionBase> actions[NUM_ENTRYPOINTS];
};
#endif
