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
#include <string>

#include <GL/glew.h>

class Sample {
public:

    virtual void startup() = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    ~Sample() {}


    static std::shared_ptr<Sample> Create(const std::string& sample);

private:
    static std::shared_ptr<Sample> Create_Simple();
    static std::shared_ptr<Sample> Create_SampleFboMSAA();
    static std::shared_ptr<Sample> Create_ShaderHandling();
 };