#ifndef DL_INTERCEPT_H
#define DL_INTERCEPT_H
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

#ifndef _WIN32
#include <map>
#include <dlfcn.h>
#include <mutex>

/**
 * Class for intercepting dynamic loader calls
 */
class DLIntercept {
   public:
    /**
     * Constructor
     */
    DLIntercept();

    /**
     * Static initializer - empty on non-Windows
     */
    static void initialize() {}

    /**
     * dlsym override implementation
     *
     * Calls real system dlsym, than calls api loader, to see,
     * if given function matches any GL API call.
     *
     * For all matching functions original pointer is stored as direct pointer
     *in api loader,
     * and wrapper pointer is returned as return value.
     *
     * For cases, when dlsym is called from GL call (catched by TLS), direct
     *pointer is returned.
     */
    void* dlsym(void* handle, const char* name);

    /**
     * dlvsym override implementation
     *
     * Calls real system dlsym, than calls api loader, to see,
     * if given function matches any GL API call.
     *
     * For all matching functions original pointer is stored as direct pointer
     *in api loader,
     * and wrapper pointer is returned as return value.
     *s
     * For cases, when dlsym is called from GL call (catched by TLS), direct
     *pointer is returned.
     */
    void* dlvsym(void* handle, const char* name, const char* version);

    /**
     * dlopen override implementation
     *
     * Calls real system dlopen, than calls api loader, to see,
     * if given library matches any known GL API library.
     *
     * For all matching libraries, the module address is stored and used
     * later for dlsym/dlvsym processing.
     *
     * On Android, for all matching libraries dglwrapper.so base address is
     *returned.
     * his is due to problems with overringing dlsym from Unity apps.
     *
     */
    void* dlopen(const char* filename, int flag);

    /**
     * Call real (not overriden) dlsym
     */
    void* real_dlsym(void* handle, const char* name);

    /**
     * Call real (not overriden) dlvsym
     */
    void* real_dlvsym(void* handle, const char* name, const char* version);

    /**
     * Call real (not overriden) dlopen
     */
    void* real_dlopen(const char* filename, int flag);

   private:
    /**
     * Utiulity function for dlsym/dlvsym override processing.
     *
     * Call matching with api loader, setting direct pointers
     * and deciding on return value happens here.
     */
    void* dlsymImpl(void* handle, const char* name, void* ptr);

    /**
     * Internal, ad-hoc initialization of class singleton
     */
    void initializeInternal();

    /**
     * Pointer to real, not overriden dlsym
     */
    void* (*m_real_dlsym)(void* handle, const char* name);

    /**
     * Pointer to real, not overriden dlvsym
     */
    void* (*m_real_dlvsym)(void* handle, const char* name, const char* version);

    /**
     * Pointer to real, not overriden dlopen
     */
    void* (*m_real_dlopen)(const char* filename, int flag);

    /**
     * Map of all matched libraries, gathered by overriding dlopen calls
     */
    std::map<uint64_t, int> mSupportedLibraries;    // handle -> ApiLibrary mask

    /**
     * True if initialized, false if initializeInternal should be called before
     * use
     */
    bool m_initialized;

    /**
     * Mutex guarding the override code
     */
    std::recursive_mutex mutex;
};

extern DLIntercept g_DLIntercept;

#ifndef NO_DL_REDEFINES
#define dlopen g_DLIntercept.real_dlopen
#define dlsym g_DLIntercept.real_dlsym
#define dlvsym g_DLIntercept.real_dlvsym
#endif

#else    // Windows implementation

#include <windows.h>
#include <mutex>

/**
 * Static class for hooking dynamic loader calls
 *
 * LoadLibraryExW is hooked - this is the kernel32.dll
 * of dynamic loader, common for all LoadLibraryA/W/ExA/ExW calls.
 */
class DLIntercept {
   public:
    /**
     * LoadLibraryExW hook implementation
     *
     * Calls real system LoadLibrary, than calls api loader, to see,
     * if given function matches any GL API library.
     *
     * All >default< GL libraries are than loaded & hooked by api loader.
     *
     * No modifications are performed to return value
     * - hooking library entry points is enough.
     */
    static HMODULE LoadLibraryExWHook(LPCWSTR lpwFileName, HANDLE hFile,
                                  DWORD dwFlags);

    /**
     * LoadLibraryExA hook implementation
     */
    static HMODULE LoadLibraryExAHook(LPCSTR lpFileName, HANDLE hFile,
                                  DWORD dwFlags);
    /**
     * LoadLibraryExW hook implementation
     */
    static HMODULE LoadLibraryWHook(LPCWSTR lpwFileName);

    /**
     * LoadLibraryExA hook implementation
     */
    static HMODULE LoadLibraryAHook(LPCSTR lpFileName);

    /**
     * Common part of hook implementation
     */
    static void LoadLibraryHookCommon(LPCSTR lpFilename);


    /**
     * Calls real, system unhooked LoadLibaryExW
     *
     * For the convenience it exposes LoadLibraryA interface.
     */
    static HMODULE real_LoadLibrary(LPCSTR lpFileName);

    /**
     * Static initializer
     *
     * Loads & hooks LoadLibraryExW function
     */
    static void initialize();

   private:
    // On windows functionality of this object is fully static
    DLIntercept();

    /**
     * typedefs for LoadLibrary* function pointers
     */
    typedef HMODULE(WINAPI *LoadLibraryA_Type)(_In_ LPCSTR lpLibFileName);
    typedef HMODULE(WINAPI *LoadLibraryW_Type)(_In_ LPCWSTR lpLibFileName);
    typedef HMODULE(WINAPI *LoadLibraryExA_Type)(_In_ LPCSTR lpLibFileName,
                                                 _Reserved_ HANDLE hFile,
                                                 _In_ DWORD dwFlags);
    typedef HMODULE(WINAPI *LoadLibraryExW_Type)(_In_ LPCWSTR lpLibFileName,
                                                 _Reserved_ HANDLE hFile,
                                                 _In_ DWORD dwFlags);

    /**
     * Pointers to real, not hooked system LoadLibrary* functions
     */
    static struct RealPtrs {
        LoadLibraryA_Type LoadLibraryA;
        LoadLibraryW_Type LoadLibraryW;
        LoadLibraryExA_Type LoadLibraryExA;
        LoadLibraryExW_Type LoadLibraryExW;
    } s_real;

    /**
     * Mutex guarding the hook code
     */
    static std::recursive_mutex s_mutex;
};

#ifndef NO_DL_REDEFINES
//These can be exported from DLIntercept, but we are lazy
#define LoadLibraryExW forbidden
#define LoadLibraryExA forbidden
#undef LoadLibraryEx
#define LoadLibraryEx  forbidden
#define LoadLibraryW   forbidden
#define LoadLibraryA   forbidden
#undef LoadLibrary
#define LoadLibrary DLIntercept::real_LoadLibrary
#endif

#endif

#endif
