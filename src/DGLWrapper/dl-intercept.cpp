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

#include <DGLCommon/os.h>

#define NO_DL_REDEFINES
#include "dl-intercept.h"

#define E std::string("ELFAnalyzer Error: ")

#include "api-loader.h"
#include "gl-wrappers.h"
#include "tls.h"
#include "wa-soctors.h"

//#define DL_INTERCEPT_DEBUG

#ifdef DL_INTERCEPT_DEBUG
#include "backtrace.h"
#endif

#ifndef __ANDROID__

class ELFAnalyzer {
   public:
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
    ~ELFAnalyzer() {
        if (m_fd > 0) {
            close(m_fd);
        }
    }

    int64_t symValue(const char *name) {
        std::map<std::string, int64_t>::iterator i = m_SymbolValues.find(name);
        if (i == m_SymbolValues.end()) {
            throw std::runtime_error(E + "Cannot find symbol " + name);
        }
        return i->second;
    }

   private:
    int m_fd;
    std::map<std::string, int64_t> m_SymbolValues;
};
#endif

DLIntercept g_DLIntercept;

DLIntercept::DLIntercept() : m_initialized(false) {}

void *DLIntercept::real_dlsym(void *handle, const char *name) {
    if (!m_initialized) {
        initialize();
    }
    return m_real_dlsym(handle, name);
}

void *DLIntercept::real_dlvsym(void *handle, const char *name,
                               const char *version) {
    if (!m_initialized) {
        initialize();
    }
    if (m_real_dlvsym) {
        return m_real_dlvsym(handle, name, version);
    } else {
        return m_real_dlsym(handle, name);
    }
}

void *DLIntercept::real_dlopen(const char *filename, int flag) {
    if (!m_initialized) {
        initialize();
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

void* DLIntercept::dlsymImpl(void* handle, const char* name, void* ptr) {
boost::recursive_mutex::scoped_lock lock(mutex);

    std::map<uint64_t, int>::iterator i =
            mSupportedLibraries.find(reinterpret_cast<uint64_t>(handle));

    Entrypoint entryp = GetEntryPointEnum(name);

    if (
        ptr &&                                          //entrypoint pointer is not NULL
        entryp != NO_ENTRYPOINT &&                      //entrypoint is supported by debugger
        !DGLThreadState::get()->inActionProcessing() && //dlsym was not emited by GL implementation
        i != mSupportedLibraries.end() &&               //library from handle is supported by debugger
        (i->second & g_ApiLoader.getEntryPointLibrary(entryp)) //library from handle match entrypoint library mask
       ) {

        //set debugger to use new entrypoint
        g_ApiLoader.setPointer(entryp,
                               reinterpret_cast<FUNC_PTR>((ptrdiff_t)ptr));


        void* ret = reinterpret_cast<void *>((ptrdiff_t)getWrapperPointer(entryp));

#ifdef DL_INTERCEPT_DEBUG
        Os::info("dlintercept: intercepted dlsym(0x%x, %s): orig 0x%x, returning 0x%x", handle, name, ptr, ret);
#endif

        //return address of a wrapper to application
        return ret;

    } else {
#ifdef DL_INTERCEPT_DEBUG
        Os::info("dlintercept: not intercepting dlsym(0x%x, %s) =  0x%x", handle, name, ptr);
#endif
        return ptr;
    }
}

void *DLIntercept::dlopen(const char *filename, int flag) {
    boost::recursive_mutex::scoped_lock lock(mutex);

    void *ret = real_dlopen(filename, flag);

    if (ret && filename) {
        int libraries = g_ApiLoader.whichLibrary(filename);
#ifdef __ANDROID__
        if (libraries & (LIBRARY_ES1 | LIBRARY_ES2)) {
            //unity3d apps on Android open libGLESvX  using dlopen,
            //but mysteriously dlsym is not visible. So on Android
            //just return libdglwrapper's base adrress to override both libs..
            Os::info("dlintercept: dlopen(%s, %d): returning dglwrapper's address.", filename, flag);
            ret = dlopen("libdglwrapper.so", RTLD_NOW);
            //we unset libraries - libdglwrapper.so cannot be marked supported.
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

void DLIntercept::initialize() {
    m_initialized = true;
#ifndef __ANDROID__
    ELFAnalyzer an("libdl");

    char *baseAddr = reinterpret_cast<char *>((intptr_t) & dlclose) -
                     an.symValue("dlclose");

    m_real_dlopen =
            reinterpret_cast<void *(*)(const char * filename, int flag)>(
                    (intptr_t)(baseAddr + an.symValue("dlopen")));
    m_real_dlsym = reinterpret_cast<void *(*)(void *, const char *)>(
            (intptr_t)(baseAddr + an.symValue("dlsym")));
    try {
        m_real_dlvsym =
                reinterpret_cast<void *(*)(void *, const char *, const char *)>(
                        (intptr_t)(baseAddr + an.symValue("dlvsym")));
    }
    catch (const std::runtime_error &err) {
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

    m_real_dlopen =
            reinterpret_cast<void *(*)(const char * filename, int flag)>(
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
 * dlopen() preloaded override.
 *
 * Called directly by debugee
 */
void *dlopen(const char *filename, int flag) NO_THROW {
#ifdef __ANDROID__
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlopen(filename, flag);
    }
    catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}

/**
 * dlsym() preloaded override.
 *
 * Called directly by debugee
 */
void *dlsym(void *handle, const char *name) NO_THROW {
#ifdef __ANDROID__
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlsym(handle, name);
    }
    catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}

/**
 * dlvsym() preloaded override.
 *
 * Called directly by debugee
 */
void *dlvsym(void *handle, const char *name, const char *version) NO_THROW {
#ifdef __ANDROID__
    DGLWASoCtors wasoCtors;
#endif
#ifdef DL_INTERCEPT_DEBUG
    Os::info(BackTrace::Get()->str().c_str());
#endif
    try {
        return g_DLIntercept.dlvsym(handle, name, version);
    }
    catch (const std::exception &e) {
        Os::fatal(e.what());
        return NULL;
    }
}
}
