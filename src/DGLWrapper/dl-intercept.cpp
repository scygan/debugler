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

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

#include<cerrno>
#include<cstring>

#include<string>
#include<stdexcept>
#include<fstream>

#ifndef __ANDROID__
#include<libelf.h>
#include<gelf.h>
#endif

#include<DGLCommon/os.h>

#define NO_DL_REDEFINES
#include"dl-intercept.h"

#define E std::string("ELFAnalyzer Error: ")

#include"api-loader.h"
#include"gl-wrappers.h"

#ifndef __ANDROID__

class ELFAnalyzer {
    public:
        ELFAnalyzer(const char* name):m_fd(-1) {
            std::string path;
            {
                std::ifstream maps("/proc/self/maps");
                std::string line;
                while (!maps.eof()) {
                    getline(maps, line);
                    std::string lineStr(line);
                    if (lineStr.find(name) != std::string::npos) {
                        path = lineStr.substr(lineStr.find_last_of( ' ' ) + 1, std::string::npos);
                        break;
                    }
                }

                if (!path.length()) {
                    throw std::runtime_error(E + "Cannot get " + name + "path when looking at /proc/self/map");
                }
            }

            if(elf_version(EV_CURRENT) == EV_NONE) {
                throw std::runtime_error(E + "libelf is outdated");
            }


            if ((m_fd = open(path.c_str(), O_RDONLY)) == -1) {
                throw std::runtime_error(E + "Cannot open: " + path + ": " + strerror(errno));
            }
            struct stat st;
            if (fstat(m_fd, &st)) {
                throw std::runtime_error(E + "Cannot stat file: " + strerror(errno));
            }

            Elf* elf = elf_begin(m_fd, ELF_C_READ, NULL);
            Elf_Scn *scn = NULL;

            GElf_Shdr shdr;

            while((scn = elf_nextscn(elf, scn)) != NULL) {
                gelf_getshdr(scn, &shdr);
                if (shdr.sh_type == SHT_DYNSYM) {
                    break;
                }
            }

            if (!scn) {
                throw std::runtime_error(E + "Cannot find DYNSYM section");
            }

            Elf_Data* edata = elf_getdata(scn, NULL);
            for(int i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
                GElf_Sym sym;
                if (!gelf_getsym(edata, i, &sym)) {
                    throw std::runtime_error(E + "gelf_getsym failed: " + elf_errmsg(elf_errno()));
                }
                if (ELF32_ST_TYPE(sym.st_info) != STT_FUNC) {
                    continue;
                }
                char* symName;
                if ((symName = elf_strptr(elf, shdr.sh_link, sym.st_name)) != NULL) {
                    m_SymbolValues[symName] = sym.st_value;
                }
            }

        }
        ~ELFAnalyzer() {
            if (m_fd > 0) {
                close(m_fd);
            }
        }

        int64_t symValue(const char* name) {
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

DLIntercept::DLIntercept():m_initialized(false) {}

void *DLIntercept::real_dlsym (void * handle, const char * name) {
    if (!m_initialized) {
        initialize();
    }
    return m_real_dlsym(handle, name);
}

void *DLIntercept::real_dlvsym (void * handle, const char *name, const char *version) {
    if (!m_initialized) {
        initialize();
    }
    if (m_real_dlvsym) {
        return m_real_dlvsym(handle, name, version);
    } else {
        return m_real_dlsym(handle, name);
    }
}

void *DLIntercept::real_dlopen (const char *filename, int flag) {
    if (!m_initialized) {
        initialize();
    }

    return m_real_dlopen(filename, flag);
}

void *DLIntercept::dlsym (void * handle, const char *name) {

    void * ptr = real_dlsym(handle, name);

    boost::recursive_mutex::scoped_lock lock(mutex);

    std::map<uint64_t, bool>::iterator i = mSupportedLibraries.find(reinterpret_cast<uint64_t>(handle));

    Entrypoint entryp = GetEntryPointEnum(name);

    if (i != mSupportedLibraries.end() && i->second && entryp != NO_ENTRYPOINT) {
        g_ApiLoader.setPointer(entryp, ptr);
        return getWrapperPointer(entryp);
    } else {
        return ptr;
    }
}

void *DLIntercept::dlvsym (void * handle, const char *name, const char *version) {

    void * ptr = real_dlvsym(handle, name, version);

    boost::recursive_mutex::scoped_lock lock(mutex);

    std::map<uint64_t, bool>::iterator i = mSupportedLibraries.find(reinterpret_cast<uint64_t>(handle));

    Entrypoint entryp = GetEntryPointEnum(name);

    if (i != mSupportedLibraries.end() && i->second && entryp != NO_ENTRYPOINT) {
        g_ApiLoader.setPointer(entryp, ptr);
        return getWrapperPointer(entryp);
    } else {
        return ptr;
    }
}

void *DLIntercept::dlopen (const char *filename, int flag) {
    boost::recursive_mutex::scoped_lock lock(mutex);

    void* ret = real_dlopen(filename, flag);

    if (filename && g_ApiLoader.isLibGL(filename)) {    
        mSupportedLibraries[reinterpret_cast<uint64_t>(ret)] = true;
    }

    return ret;
}


void DLIntercept::initialize() {
    m_initialized = true;
#ifndef __ANDROID__
    ELFAnalyzer an("libdl");

    char* baseAddr = reinterpret_cast<char*>(&dlclose) - an.symValue("dlclose");

    m_real_dlopen = reinterpret_cast<void* (*)(const char *filename, int flag)> (
            baseAddr + an.symValue("dlopen"));
    m_real_dlsym = reinterpret_cast<void* (*)(void*, const char*)> (
            baseAddr + an.symValue("dlsym"));
    try {
        m_real_dlvsym = reinterpret_cast<void* (*)(void*, const char*, const char*)>(
            baseAddr + an.symValue("dlvsym"));
    } catch (const std::runtime_error& err) {        Os::nonFatal("dlvsym is not available in dynamic linker.\n");
        m_real_dlvsym = NULL;
    }
#else
    //dlopen/dlsym is injected in /system/bin/linker on Android from local functions (and libdl.so is only a stub).
    //There is no way to get addresses of real dlopen/dlsym via elf parsing, as linker has -fvisibility=hidden.

    //We strongly rely on DGLLoader to get those symbols..

    char* baseAddr = reinterpret_cast<char*>(&dlclose);

    m_real_dlopen = reinterpret_cast<void* (*)(const char *filename, int flag)> (
        baseAddr + atoi(Os::getEnv("dlopen_addr").c_str()));

    m_real_dlsym = reinterpret_cast<void* (*)(void*, const char*)> (
        baseAddr + atoi(Os::getEnv("dlsym_addr").c_str()));

    m_real_dlvsym = NULL;

#endif

}

extern "C" {

    //these are preloaded entrypoints called by implementation


    /** 
     * dlopen() preloaded override.
     *
     * Called directly by debugee
     */
    void *dlopen(const char *filename, int flag) {
        try {
            return g_DLIntercept.dlopen(filename, flag);
        } catch (const std::exception& e) {
            Os::fatal(e.what());
            return NULL;
        }
    }

    /** 
     * dlsym() preloaded override.
     *
     * Called directly by debugee
     */
    void *dlsym (void * handle, const char *name) {
        try {
            return g_DLIntercept.dlsym(handle, name);
        } catch (const std::exception& e) {
            Os::fatal(e.what());
            return NULL;
        }
    }

    /** 
     * dlvsym() preloaded override.
     *
     * Called directly by debugee
     */
    void *dlvsym (void * handle, const char *name, const char *version) {
        try {
            return g_DLIntercept.dlvsym(handle, name, version);
        } catch (const std::exception& e) {
            Os::fatal(e.what());
            return NULL;
        }
    }
}
