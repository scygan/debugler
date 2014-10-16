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
#include <DGLCommon/def.h>

class OsIcon {
   public:
    virtual void* get() = 0;
    virtual ~OsIcon() {}
};

class OsStatusPresenter {
   public:
    virtual void setStatus(const std::string& message) = 0;
    virtual ~OsStatusPresenter() {}
};

class Os {
   public:
    static int getProcessPid();

    static std::string getProcessName();

    static std::string getEnv(const char* variable);

    static void setEnv(const char* variable, const char* value);

#ifdef __ANDROID__
    static std::string getProp(const char* property);

    static bool setProp(const char* property, const char* value);
#endif 

    NO_RETURN static void fatal(const char* fmt, ...);

    static void nonFatal(const char* fmt, ...);
    static void info(const char* fmt, ...);

#ifndef NDEBUG
    static void debug(const char* fmt, ...);
#endif

    NO_RETURN static void terminate();

    static OsStatusPresenter* createStatusPresenter();

    static OsIcon* createIcon();

    static void setCurrentModuleHandle(void* handle);
    static void* getCurrentModuleHandle();

    static int getLastosError();

    static std::string translateOsError(int error);

   private:
    static void* m_CurrentHandle;

    static std::string vargsToString(const char* fmt, va_list arg);
};

#ifdef NDEBUG
#define OS_DEBUG(...)
#else
#define OS_DEBUG(...) Os::debug(__VA_ARGS__)
#endif

#endif
