#ifndef DL_INTERCEPT_H
#define DL_INTERCEPT_H
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

#include <boost/thread/recursive_mutex.hpp>

#ifndef _WIN32
#include <map>

#include <dlfcn.h>

class DLIntercept {
   public:
    DLIntercept();

    static void initialize() {}

    void* dlsym(void* handle, const char* name);
    void* dlvsym(void* handle, const char* name, const char* version);
    void* dlopen(const char* filename, int flag);

    void* real_dlsym(void* handle, const char* name);
    void* real_dlvsym(void* handle, const char* name, const char* version);
    void* real_dlopen(const char* filename, int flag);

   private:
    void* dlsymImpl(void* handle, const char* name, void* ptr);
    void initializeInternal();

    void* (*m_real_dlsym)(void* handle, const char* name);
    void* (*m_real_dlvsym)(void* handle, const char* name, const char* version);
    void* (*m_real_dlopen)(const char* filename, int flag);

    std::map<uint64_t, int> mSupportedLibraries; //handle -> ApiLibrary mask

    bool m_initialized;
    boost::recursive_mutex mutex;
};

extern DLIntercept g_DLIntercept;

#ifndef NO_DL_REDEFINES
#define dlopen g_DLIntercept.real_dlopen
#define dlsym g_DLIntercept.real_dlsym
#define dlvsym g_DLIntercept.real_dlvsym
#endif

#else //Windows implementation

#include<windows.h>

class DLIntercept {
public:

    static HMODULE  LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);

    static HMODULE  real_LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
    
    static HMODULE  real_LoadLibrary(LPCSTR lpFileName);

    static void initialize();

private:
    //On windows functionality of this object is fully static
    DLIntercept();
    
    typedef HMODULE (WINAPI *LoadLibraryExW_Type)( _In_ LPCWSTR lpFileName, _Reserved_  HANDLE hFile, _In_ DWORD dwFlags);

    static LoadLibraryExW_Type s_real_LoadLibraryExW;

    static boost::recursive_mutex s_mutex;
};

#ifndef NO_DL_REDEFINES
#define LoadLibraryExW DLIntercept::real_LoadLibraryExW
#undef LoadLibrary
#define LoadLibrary    DLIntercept::real_LoadLibrary
#endif


#endif

#endif
