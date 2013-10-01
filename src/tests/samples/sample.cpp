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

#include "sample.h"

#include <stdexcept>


std::map<std::string, std::shared_ptr<Sample>> * Sample::getRegistry() {
    static std::map<std::string, std::shared_ptr<Sample>> * s_Registry = new std::map<std::string, std::shared_ptr<Sample>>();
    return s_Registry;
}

bool Sample::registerSample(Sample* sample, const std::string& name) {
    (*getRegistry())[name] = std::shared_ptr<Sample>(sample);
    return true;
}

std::shared_ptr<Sample> Sample::getSample(const std::string& sample) {
    auto res = getRegistry()->find(sample);
    if (res == getRegistry()->end()) {
        throw std::runtime_error("Unknown sample to run");
    }
    return res->second;
}
