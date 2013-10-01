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
#include <map>

#include <GL/glew.h>

class Sample {
public:

    virtual void startup() = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    ~Sample() {}


    static std::shared_ptr<Sample> getSample(const std::string& sample);


    static bool registerSample(Sample* sample, const std::string& name);
private:

    static std::map<std::string, std::shared_ptr<Sample>> * getRegistry();
    
};


#define REGISTER_SAMPLE(CLASS, NAME) \
    bool CLASS##registeredInfo =  CLASS::registerSample(new CLASS(), NAME)
    