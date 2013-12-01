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

#include "os.h"
#include <vector>
#include <cstdarg>
#include <cerrno>
#ifdef _WIN32
#define va_copy(dest, src) (dest = src)
#else
#include <cstring>
#endif

#ifdef _WIN32
#undef WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>

#include "resource.h"

class OsIconImpl : public OsIcon {
   public:
    OsIconImpl(void* moduleHandle) {
        m_Icon = (HICON)LoadImage((HMODULE)moduleHandle,
                                  MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0,
                                  LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
    }

    virtual ~OsIconImpl() { DestroyIcon(m_Icon); }

    virtual void* get() { return m_Icon; }
    static void* m_Handle;

   private:
    HICON m_Icon;
};

class OsStatusPresenterImpl : public OsStatusPresenter {
   public:
    OsStatusPresenterImpl(void* moduleHandle) : m_Icon(moduleHandle) {
        memset(&m_niData, 0, sizeof(m_niData));
        m_niData.cbSize = sizeof(m_niData);
        m_niData.hWnd = GetDesktopWindow();
        m_niData.uID = 0xdeb091e4;
        m_niData.uTimeout = 5000;    // deprecated on Vista
        m_niData.hIcon = (HICON)m_Icon.get();
        m_niData.uFlags = NIF_ICON;

        std::string process = Os::getProcessName();
        memcpy(m_niData.szInfoTitle, process.c_str(), process.length() + 1);

        Shell_NotifyIcon(NIM_ADD, &m_niData);
    }
    virtual void setStatus(const std::string message) {
        memcpy(m_niData.szInfo, message.c_str(), message.length() + 1);
        m_niData.uFlags |= NIF_INFO;
        Shell_NotifyIcon(NIM_MODIFY, &m_niData);
    }
    virtual ~OsStatusPresenterImpl() {
        Shell_NotifyIcon(NIM_DELETE, &m_niData);
    }

   private:
    NOTIFYICONDATA m_niData;
    OsIconImpl m_Icon;
};

int Os::getProcessPid() { return (int)GetProcessId(GetCurrentProcess()); }

std::string Os::getProcessName() {
    std::string ret = "<unknown>";

    HANDLE currentProcess = GetCurrentProcess();
    if (currentProcess) {
        std::vector<char> buff(200);
        buff[GetModuleBaseName(currentProcess, NULL, &buff[0], 200)] = 0;
        if (buff[0]) {
            ret = &buff[0];
        }
    }
    return ret;
}

std::string Os::getEnv(const char* variable) {
    char ret[100];
    if (GetEnvironmentVariable(variable, ret, sizeof(ret)) != 0) {
        return ret;
    }
    return "";
}

void Os::setEnv(const char* variable, const char* value) {
    SetEnvironmentVariable(variable, value);
}

void Os::fatal(const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::string message = vargsToString(fmt, args);
    va_end(args);

    MessageBox(0, message.c_str(), "Fatal debugger error", MB_OK | MB_ICONSTOP);
    fprintf(stderr, "Error: %s\n", message.c_str());
    exit(EXIT_FAILURE);
}

void Os::terminate() { TerminateProcess(GetCurrentProcess(), 0); }

OsStatusPresenter* Os::createStatusPresenter() {
    if (!m_CurrentHandle) {
        m_CurrentHandle = GetModuleHandle(NULL);
    }
    return new OsStatusPresenterImpl(m_CurrentHandle);
}

OsIcon* Os::createIcon() {
    if (!m_CurrentHandle) {
        m_CurrentHandle = GetModuleHandle(NULL);
    }
    return new OsIconImpl(m_CurrentHandle);
}

void Os::setCurrentModuleHandle(void* handle) { m_CurrentHandle = handle; }

void* Os::m_CurrentHandle = NULL;

int Os::getLastosError() { return (int)GetLastError(); }

std::string Os::translateOsError(int error) {
    char* errorText;
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPSTR) & errorText, 0, NULL);
    std::string ret(errorText);
    LocalFree(errorText);
    return ret;
}

#else

#include <cstdio>
#include <cstdlib>
#include <stdexcept>    //remove me
#include <unistd.h>
#include <libgen.h>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "Debugler"
#endif

class OsIconImpl : public OsIcon {
   public:
    OsIconImpl() {}
    virtual ~OsIconImpl() {}

    virtual void* get() {
        // need to actually implement window icon here
        throw std::runtime_error("Not implemented");
    }
};

OsIcon* Os::createIcon() { return new OsIconImpl(); }

void Os::fatal(const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::string message = vargsToString(fmt, args);
    va_end(args);

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, "%s", message.c_str());
#endif
    fprintf(stderr, "Error: %s\n", message.c_str());
    exit(EXIT_FAILURE);
}

void Os::setEnv(const char* variable, const char* value) {
    setenv(variable, value, 1);
}

std::string Os::getEnv(const char* variable) {
    char* ret = getenv(variable);
    if (ret) {
        return ret;
    }
    return "";
}

int Os::getProcessPid() { return getpid(); }

std::string Os::getProcessName() {
    std::string ret = "<unknown>";
    size_t linknamelen;
    char cmdline[256] = {0};
    const char* file = "/proc/self/exe";
    linknamelen =
            readlink(file, cmdline, sizeof(cmdline) / sizeof(*cmdline) - 1);
    cmdline[linknamelen + 1] = 0;
    return basename(cmdline);
}

void Os::terminate() { _exit(0); }

class OsStatusPresenterImpl : public OsStatusPresenter {
   public:
    virtual void setStatus(const std::string message) {
        printf("DGLWrapper: %s\n", message.c_str());
    }
    virtual ~OsStatusPresenterImpl() {}

   private:
};

OsStatusPresenter* Os::createStatusPresenter() {
    return new OsStatusPresenterImpl();
}

int Os::getLastosError() { return errno; }

std::string Os::translateOsError(int error) { return strerror(error); }

#endif

void Os::nonFatal(const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::string message = vargsToString(fmt, args);
    va_end(args);

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", message.c_str());
#endif
    fprintf(stderr, "Warning: %s\n", message.c_str());
}

void Os::info(const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::string message = vargsToString(fmt, args);
    va_end(args);

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", message.c_str());
#endif
    fprintf(stdout, "%s\n", message.c_str());
}

#ifndef NDEBUG
void Os::debug(const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);
    std::string message = vargsToString(fmt, args);
    va_end(args);

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Debug %s", message.c_str());
#endif
    fprintf(stdout, "Debug: %s\n", message.c_str());
}
#endif

std::string Os::vargsToString(const char* fmt, va_list args) {

    va_list argsCopy;
    va_copy(argsCopy, args);
#ifdef _WIN32
    size_t length = _vscprintf(fmt, argsCopy);
#else
    size_t length = vsnprintf(NULL, 0, fmt, argsCopy);
#endif
    va_end(argsCopy);

    std::vector<char> buff(length + 1);

#ifdef _WIN32
    vsnprintf_s(&buff[0], buff.size(), _TRUNCATE, fmt, args);
#else
    vsnprintf(&buff[0], buff.size(), fmt, args);
#endif
    return std::string(&buff[0]);
}
