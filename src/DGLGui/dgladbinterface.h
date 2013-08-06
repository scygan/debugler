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


#ifndef DGLADBINTERFACE_H
#define DGLADBINTERFACE_H


#include <memory>
#include <string>
#include <vector>

class DGLAdbDevice {

};


class DGLAdbInterface {
public:
    
    static DGLAdbInterface* get();

    void setAdbPath(std::string path);
    std::string getAdbPath();

    void killServer();
    void connect(std::string address);

    std::vector<DGLAdbDevice> getDevices();

private:

    void invokeAdb(std::vector<std::string> params);

    std::string m_adbPath;

    static std::shared_ptr<DGLAdbInterface> s_self;
};

#endif
