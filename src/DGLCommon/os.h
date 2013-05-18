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


#ifndef OS_H
#define OS_H

#include <string>

#ifdef _WIN32
#define NO_RETURN __declspec(noreturn) 
#else
#define NO_RETURN __attribute__((noreturn))
#endif

class OsIcon {
public:
    virtual void * get() = 0;
};

class OsStatusPresenter {
public:
    virtual void setStatus(const std::string message) = 0;
    virtual ~OsStatusPresenter() {}
};

class Os {
public:
    static std::string getProcessName(); 

    static std::string getEnv(const char* variable);

    static void setEnv(const char* variable, const char* value);

    NO_RETURN static void fatal(const std::string& message);

    static void nonFatal(const std::string& message);

    NO_RETURN static void terminate();

    static OsStatusPresenter* createStatusPresenter();

    static OsIcon* createIcon();

    static void setCurrentModuleHandle(void * handle);
private:
    static void* m_CurrentHandle;
};



#endif

