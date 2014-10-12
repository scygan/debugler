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

#include "dl.h"

#include <DGLCommon/os.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//for real dlsym/LoadLibrary
#include "dl-intercept.h"

#include <sstream>

#ifdef _WIN32
class DynamicLibraryImpl: public DynamicLibrary {
public:
    DynamicLibraryImpl(const char* name) {
        m_Module = LoadLibrary(name);
        if (!m_Module) {
            std::ostringstream error;
            error << "Cannot open library " << name << ".";
            throw std::runtime_error(error.str());
        }
    }
    virtual ~DynamicLibraryImpl() {
        if (m_Module) {
            FreeLibrary(m_Module);
        }
    }
    virtual dgl_func_ptr getFunction(const char* symbolName) const override {
        return reinterpret_cast<dgl_func_ptr>(GetProcAddress(m_Module, symbolName));
    }
private:
    HMODULE m_Module;
};
#else //_WIN32
class DynamicLibraryImpl: public DynamicLibrary {
public:
    DynamicLibraryImpl(const char* name) {
        m_Handle = dlopen(name, RTLD_NOW);
        if (!m_Handle) {
            std::ostringstream error;
            error << "Cannot open library " << name << ".";
            throw std::runtime_error(error.str());
        }
    }
    virtual ~DynamicLibraryImpl() {
        if (m_Handle) {
            dlclose(m_Handle);
        }
    }
    virtual dgl_func_ptr getFunction(const char* symbolName) const override {
        //(ptrdiff_t) -> see http://www.trilithium.com/johan/2004/12/problem-with-dlsym/
        return reinterpret_cast<dgl_func_ptr>(
            (ptrdiff_t)dlsym(m_Handle, symbolName));
    }
private:
    void* m_Handle;
};
#endif

std::shared_ptr<DynamicLibrary> DynamicLoader::getLibrary(const char* name) {
   std::map<std::string, std::shared_ptr<DynamicLibrary> >::iterator i = m_OpenLibraries.find(name);
    if (i != m_OpenLibraries.end()) {
        return i->second;
    } else {
        try {
            std::shared_ptr<DynamicLibrary> ret =  std::make_shared<DynamicLibraryImpl>(name);
            m_OpenLibraries[name] = ret;
            return ret;
        } catch (const std::runtime_error& e) {
            Os::nonFatal(e.what());
            return nullptr;
        }
    }
}

#ifndef _WIN32
std::string DynamicLoader::getCurrentLibraryName() {
    Dl_info currentLibraryInfo;
    dladdr(&s_DummySymbol, &currentLibraryInfo);
    return currentLibraryInfo.dli_fname;
}

void* DynamicLoader::getCurrentLibraryBaseAddress() {
    Dl_info currentLibraryInfo;
    dladdr(&s_DummySymbol, &currentLibraryInfo);
    return currentLibraryInfo.dli_fbase;
}
#endif

int DynamicLoader::s_DummySymbol;
