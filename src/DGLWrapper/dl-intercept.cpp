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

#define NO_DL_REDEFINES
#include "dl-intercept.h"
#include "globalstate.h"
#include "api-loader.h"
#include <DGLCommon/os.h>

#ifndef _WIN32

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <cerrno>
#include <cstring>

#include <string>
#include <stdexcept>
#include <fstream>

#ifndef __ANDROID__
#include <libelf.h>
#include <gelf.h>
#else
#include "ipc.h"
#endif

#define E std::string("ELFAnalyzer Error: ")

#include "gl-wrappers.h"
#include "tls.h"
#include "wa-soctors.h"

//#define DL_INTERCEPT_DEBUG

#ifdef DL_INTERCEPT_DEBUG
#include "backtrace.h"
#endif

#ifndef __ANDROID__

/**
 * Utility class for discovering symbol offsets from SO files
 * Used on Linux to get dynamic linker entrypoints, which are normally
 *  unavailable because of LD_PRELOAD override.
 *
 * libelf is used under the hood.
 * Not used on Android, because dlsym/dlopen are hidden symbols on this system,
 * added manually to process symbol list by /system/bin/linker.
 */
class ELFAnalyzer {
   public:
    /**
     * Countructor
     *
     * Opens the library and parses it's DYNSYM
     */
    ELFAnalyzer(const char *name) : m_fd(-1) {
        std::string path;
        {
            std::ifstream maps("/proc/self/maps");
            std::string line;
            while (!maps.eof()) {
                getline(maps, line);
                std::string lineStr(line);
                if (lineStr.find(name) != std::string::npos) {
                    path = lineStr.substr(lineStr.find_last_of(' ') + 1,
                                          std::string::npos);
                    break;
                }
            }

            if (!path.length()) {
                throw std::runtime_error(E + "Cannot get " + name +
                                         "path when looking at /proc/self/map");
            }
        }

        if (elf_version(EV_CURRENT) == EV_NONE) {
            throw std::runtime_error(E + "libelf is outdated");
        }

        if ((m_fd = open(path.c_str(), O_RDONLY)) == -1) {
            throw std::runtime_error(E + "Cannot open: " + path + ": " +
                                     strerror(errno));
        }
        struct stat st;
        if (fstat(m_fd, &st)) {
            throw std::runtime_error(E + "Cannot stat file: " +
                                     strerror(errno));
        }

        Elf *elf = elf_begin(m_fd, ELF_C_READ, NULL);
        Elf_Scn *scn = NULL;

        GElf_Shdr shdr;

        while ((scn = elf_nextscn(elf, scn)) != NULL) {
            gelf_getshdr(scn, &shdr);
            if (shdr.sh_type == SHT_DYNSYM) {
                break;
            }
        }

        if (!scn) {
            throw std::runtime_error(E + "Cannot find DYNSYM section");
        }

        Elf_Data *edata = elf_getdata(scn, NULL);
        for (size_t i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
            GElf_Sym sym;
            if (!gelf_getsym(edata, i, &sym)) {
                throw std::runtime_error(E + "gelf_getsym failed: " +
                                         elf_errmsg(elf_errno()));
            }
            if (ELF32_ST_TYPE(sym.st_info) != STT_FUNC) {
                continue;
            }
            char *symName;
            if ((symName = elf_strptr(elf, shdr.sh_link, sym.st_name)) !=
                NULL) {
                m_SymbolValues[symName] = sym.st_value;
            }
        }
    }

    /**
     * Destructor
     *
     * Closes the library
     */
    ~ELFAnalyzer() {
        if (m_fd > 0) {
            close(m_fd);
        }
    }

    /**
     * Get offset of symbol from library
     *
     * @param name name of the symbol
     * @returns offset, in bytes of given symbol
     */
    int64_t symValue(const char *name) {
        std::map<std::string, int64_t>::iterator i = m_SymbolValues.find(name);
        if (i == m_SymbolValues.end()) {
            throw std::runtime_error(E + "Cannot find symbol " + name);
        }
        return i->second;
    }

   private:
    /**
     * The file handle of opened library
     */
    int m_fd;
    /**
     * Map of all offsets of DYNSYM symbols
     */
    std::map<std::string, int64_t> m_SymbolValues;
};
#endif
/**
 * Global instance of DLIntercept object
 *
 * TODO: change to static class?
 */
DLIntercept g_DLIntercept;

DLIntercept::DLIntercept() : m_initialized(false) {}

void *DLIntercept::real_dlsym(void *handle, const char *name) {
    if (!m_initialized) {
        initializeInternal();
    }
    return m_real_dlsym(handle, name);
}

void *DLIntercept::real_dlvsym(void *handle, const char *name,
                               const char *version) {
    if (!m_initialized) {
        initializeInternal();
    }
    if (m_real_dlvsym) {
        return m_real_dlvsym(handle, name, version);
    } else {
        return m_real_dlsym(handle, name);
    }
}

void *DLIntercept::real_dlopen(const char *filename, int flag) {
    if (!m_initialized) {
        initializeInternal();
    }

    return m_real_dlopen(filename, flag);
}

void *DLIntercept::dlsym(void *handle, const char *name) {

    void *ptr = real_dlsym(handle, name);

    return dlsymImpl(handle, name, ptr);
}

void *DLIntercept::dlvsym(void *handle, const char *name, const char *version) {

    void *ptr = real_dlvsym(handle, name, version);

    return dlsymImpl(handle, name, ptr);
}

void *DLIntercept::dlsymImpl(void *handle, const char *name, void *ptr) {
    boost::recursive_mutex::scoped_lock lock(mutex);

    std::map<uint64_t, int>::iterator i =
            mSupportedLibraries.find(reinterpret_cast<uint64_t>(handle));

    Entrypoint entryp = GetEntryPointEnum(name);

    if (ptr &&                        // entrypoint pointer is not NULL
        entryp != NO_ENTRYPOINT &&    // entrypoint is supported by debugger
        !DGLThreadState::get()->inActionProcessing() &&    // dlsym was not
                                                           // emited by GL
                                                           // implementation
        i != mSupportedLibraries.end() &&    // library from handle is supported
                                             // by debugger
        (i->second &
         EarlyGlobalState::getApiLoader().getEntryPointLibrary(
                 entryp))    // library from handle match entrypoint library
                             // mask
        ) {

        // set debugger to use new entrypoint
        EarlyGlobalState::getApiLoader().setPointer(
                entryp, reinterpret_cast<dgl_func_ptr>((ptrdiff_t)ptr));

        void *ret =
                reinterpret_cast<void *>((ptrdiff_t)getWrapperPointer(entryp));

#ifdef DL_INTERCEPT_DEBUG
        Os::info(
                "dlintercept: intercepted dlsym(0x%x, %s): orig 0x%x, "
                "returning 0x%x",
                handle, name, ptr, ret);
#endif

        // return address of a wrapper to application
        return ret;

    } else {
#ifdef DL_INTERCEPT_DEBUG
        Os::info("dlintercept: not intercepting dlsym(0x%x, %s) =  0x%x",
                 handle, name, ptr);
#endif
        return ptr;
    }
}

void *DLIntercept::dlopen(const char *filename, int flag) {
    boost::recursive_mutex::scoped_lock lock(mutex);

    void *ret = real_dlopen(filename, flag);

    if (ret && filename) {
        int libraries = EarlyGlobalState::getApiLoader().whichLibrary(filename);
#ifdef __ANDROID__
        if (libraries & (LIBRARY_ES1 |
                         LIBRARY_ES2) &&    // library is affected coer library
            !DGLThreadState::get()->inActionProcessing()    // call is emmitted
                                                            // by app, not by GL
                                                            // or DGL.
            ) {

            /* Unity3d apps on Android open libGLESvX  using dlopen,
            but mysteriously dlsym is not visible. So on Android
            just return libdglwrapper's base adrress to override both libs.. */

            Os::info(
                    "dlintercept: dlopen(%s, %d): returning dglwrapper's "
                    "address.",
                    filename, flag);
            ret = dlopen(DynamicLoader::getCurrentLibraryName().c_str(), RTLD_NOW);
            // we unset libraries - libdglwrapper.so cannot be marked supported.
            libraries = LIBRARY_NONE;
        }
#endif
        if (libraries != LIBRARY_NONE) {
            mSupportedLibraries[reinterpret_cast<uint64_t>(ret)] = libraries;
        }
    }

#ifdef DL_INTERCEPT_DEBUG
    Os::info("dlintercept: dlopen(%s, %d) = 0x%x", filename, flag, ret);
#endif

    return ret;
}

void DLIntercept::initializeInternal() {
    m_initialized = true;
#ifndef __ANDROID__
    ELFAnalyzer an("libdl");

    char *baseAddr = reinterpret_cast<char *>((intptr_t)&dlclose) -
                     an.symValue("dlclose");

    m_real_dlopen = reinterpret_cast<void *(*)(const char *filename, int flag)>(
            (intptr_t)(baseAddr + an.symValue("dlopen")));
    m_real_dlsym = reinterpret_cast<void *(*)(void *, const char *)>(
            (intptr_t)(baseAddr + an.symValue("dlsym")));
    try {
        m_real_dlvsym =
                reinterpret_cast<void *(*)(void *, const char *, const char *)>(
                        (intptr_t)(baseAddr + an.symValue("dlvsym")));
    } catch (const std::runtime_error &err) {
        Os::nonFatal("dlvsym is not available in dynamic linker.\n");
        m_real_dlvsym = NULL;
    }
#else
    // dlopen/dlsym is injected in /system/bin/linker on Android from local
    // functions (and libdl.so is only a stub).
    // There is no way to get addresses of real dlopen/dlsym via elf parsing, as
    // linker has -fvisibility=hidden.

    // We strongly rely on DGLLoader to get those symbols..

    char *baseAddr =
            reinterpret_cast<char *>(reinterpret_cast<intptr_t>(&dlclose));

    int dlOpenAddr, dlSymAddr;
    getIPC()->getDLInternceptPointers(dlOpenAddr, dlSymAddr);

    m_real_dlopen = reinterpret_cast<void *(*)(const char *filename, int flag)>(
            reinterpret_cast<intptr_t>(baseAddr + dlOpenAddr));

    m_real_dlsym = reinterpret_cast<void *(*)(void *, const char *)>(
            reinterpret_cast<intptr_t>(baseAddr + dlSymAddr));

    m_real_dlvsym = NULL;
#endif
}

extern "C" {

#ifndef __ANDROID__
#define NO_THROW throw()
#else
// Android does not have throw() directive in dlfnc declarations
#define NO_THROW
#endif

// these are preloaded entrypoints called by implementation

/**
 * dlopen() entrypoint, preloaded override.
 *
 * Called directly by debugee
 */
void *dlopen(const char *filename, int flag) NO_THROW {
#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlopen(filename, flag);
    } catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}

/**
 * dlsym() entrypoint, preloaded override.
 *
 * Called directly by debugee
 */
void *dlsym(void *handle, const char *name) NO_THROW {
#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlsym(handle, name);
    } catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}

/**
 * dlvsym() entrypoint, preloaded override.
 *
 * Called directly by debugee
 */
void *dlvsym(void *handle, const char *name, const char *version) NO_THROW {
#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlvsym(handle, name, version);
    } catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}
}    // extern "C"

#else    // Windows implementation follows.

#include <vector>
#include "ipc.h"
#include "hook.h"

HMODULE DLIntercept::LoadLibraryExW(_In_ LPCWSTR lpwFileName,
                                    _Reserved_ HANDLE hFile,
                                    _In_ DWORD dwFlags) {
    boost::recursive_mutex::scoped_lock lock(s_mutex);

    HMODULE ret = real_LoadLibraryExW(lpwFileName, hFile, dwFlags);

    if (ret && lpwFileName) {

        std::vector<char> fileName;
        {
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, lpwFileName, -1,
                                                  NULL, 0, NULL, NULL);
            fileName.resize(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, lpwFileName, -1, &fileName[0],
                                size_needed, NULL, NULL);
        }

        int libraries =
                EarlyGlobalState::getApiLoader().whichLibrary(&fileName[0]);
        EarlyGlobalState::getApiLoader().loadDefaultLibraries(
                getIPC()->getDebuggerMode() == DGLIPC::DebuggerMode::EGL,
                libraries, APILoader::LoadMode::IMMEDIATE);
    }
    return ret;
}

HMODULE WINAPI LoadLibraryExW_Call(_In_ LPCWSTR lpwFileName,
                                   _Reserved_ HANDLE hFile,
                                   _In_ DWORD dwFlags) {
    try {
        return DLIntercept::LoadLibraryExW(lpwFileName, hFile, dwFlags);
    } catch (const std::exception &e) {
        Os::fatal(e.what());
    }
}

HMODULE DLIntercept::real_LoadLibraryExW(LPCWSTR lpwFileName, HANDLE hFile,
                                         DWORD dwFlags) {
    return s_real_LoadLibraryExW(lpwFileName, hFile, dwFlags);
}

HMODULE DLIntercept::real_LoadLibrary(LPCSTR lpFileName) {

    std::vector<char> wfileName;
    {
        int size_needed =
                MultiByteToWideChar(CP_UTF8, 0, lpFileName, -1, NULL, 0);
        wfileName.resize(size_needed * 2, 0);
        MultiByteToWideChar(CP_UTF8, 0, lpFileName, -1,
                            reinterpret_cast<LPWSTR>(&wfileName[0]),
                            size_needed);
    }
    return real_LoadLibraryExW(reinterpret_cast<LPWSTR>(&wfileName[0]), nullptr,
                               0);
}

void DLIntercept::initialize() {

    HMODULE kernel32Module = LoadLibrary("kernel32.dll");

    s_real_LoadLibraryExW = reinterpret_cast<LoadLibraryExW_Type>(
            GetProcAddress(kernel32Module, "LoadLibraryExW"));

    if (s_real_LoadLibraryExW) {

        HookSession hookSession;

        dgl_func_ptr hookPtr = reinterpret_cast<dgl_func_ptr>(&LoadLibraryExW_Call);
        hookSession.hook((dgl_func_ptr *)&s_real_LoadLibraryExW,
                         hookPtr);
    }
}

DLIntercept::LoadLibraryExW_Type DLIntercept::s_real_LoadLibraryExW;

boost::recursive_mutex DLIntercept::s_mutex;

#endif
