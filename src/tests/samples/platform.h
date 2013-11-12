#ifndef PLATFORM_H
#define PLATFORM_H
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

#include <memory>

class PlatWindowCtx {
   public:
    virtual void makeCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual bool pendingClose() = 0;
    virtual void resize(int newWidth, int newHeight) = 0;
};

class Platform {
   public:
    Platform();
    ~Platform();

    std::shared_ptr<PlatWindowCtx> createWindow();

    void pollEvents();
};
#endif