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

#include <DGLCommon/wa.h>

#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
#include <DGLCommon/os.h>
#include "wa-soctors.h"
#include "dl-intercept.h" //redef dlopen
#include "dl.h"
#include <mutex>
#include <vector>
#include <elf.h>
#include <dlfcn.h>

void __attribute__((destructor)) InitializersRunOnDestructionEnforcer(void) {
    // this is the first static destructor called in application (depends on
    // CMakeLists.txt).

    // this ensures all contructors are called before destructors, if nobody
    // cared to call them earlier.

    DGLWASoCtors soCtorsWa;
}

class StaticInitializerTest {
   public:
    StaticInitializerTest() {
        // something not so trivial for the compiler:
        m_TestVector.resize(3);
    }

    bool passed() { return m_TestVector.size() == 3; }

   private:
    std::vector<int> m_TestVector;

} g_Test;

std::mutex* g_mutex = NULL;

// this is from /bionic/linker/linker.h
#define SOINFO_NAME_LEN 128
struct soinfo {
    const char name[SOINFO_NAME_LEN];
    Elf32_Phdr *phdr;
    int phnum;
    unsigned entry;
    unsigned base;
    // some more interesting stuff is here, but
    // we want to compute these manually, as soinfo layout may change in future.
};

#ifndef DT_INIT_ARRAY
#define DT_INIT_ARRAY 25
#endif

#ifndef DT_INIT_ARRAYSZ
#define DT_INIT_ARRAYSZ 27
#endif

DGLWASoCtors::DGLWASoCtors() {


    if (g_Test.passed()) {
        return;
    }

    if (!g_mutex) {
        //as initializers where not called, mutex must be constructed manually on the heap
        g_mutex = new std::mutex();
    }

    {
        std::lock_guard<std::mutex> guard(*g_mutex);

        //check again, this is essential for threads that were waiting.
        //Otherwise will call ctors multiple times.
        if (g_Test.passed()) {
            return;
        }

        Os::info(
            "Failed shared library constructor test. This is typical on "
            "Android < "
            "4.2-r1");
        Os::info("Will try to run constructors manually now.");

        std::string currentLibraryName = DynamicLoader::getCurrentLibraryName();

        soinfo *info =
            reinterpret_cast<soinfo *>(dlopen(currentLibraryName.c_str(), RTLD_NOW));

        if (!info) {
            Os::fatal("Cannot dlopen %d library", currentLibraryName.c_str());
        }

        unsigned *dynamic = nullptr;

        Elf32_Phdr *phdr = info->phdr;
        int phnum = info->phnum;

        Os::info(
            "Trying to get .dynamic of %s: base = 0x%x, phnum = "
            "%d",
            currentLibraryName.c_str(), info->base, info->phnum);

        for (; phnum > 0; --phnum, ++phdr) {
            if (phdr->p_type == PT_DYNAMIC) {
                dynamic = (unsigned *)(info->base + phdr->p_vaddr);
            }
        }

        if (!dynamic || dynamic == (unsigned *)-1) {
            Os::fatal("Cannot get .dynamic section of %s.", currentLibraryName.c_str());
        } else {
            Os::info("Found .dynamic at 0x%x", dynamic);
        }

        void (*init_func)(void) = nullptr;
        unsigned *init_array = nullptr;
        unsigned init_array_count = 0;

        for (unsigned *d = dynamic; *d; d++) {
            switch (*d++) {
            case DT_INIT:
                init_func = (void (*)(void))(info->base + *d);
                break;
            case DT_INIT_ARRAYSZ:
                init_array_count = ((unsigned)*d) / sizeof(Elf32_Addr);
                break;
            case DT_INIT_ARRAY:
                init_array = (unsigned *)(info->base + *d);
                break;
            }
        }

        if (init_func) {
            Os::info("Found DT_INIT pointing at address 0x%x, Calling it",
                init_func);
            init_func();
        }
        if (init_array_count && init_array) {
            Os::info("Found DT_INIT_ARRAY of size %d", init_array_count);
            for (unsigned i = 0; i < init_array_count; i++) {
                if (init_array[i] && init_array[i] != (unsigned)-1) {
                    OS_DEBUG("Calling DT_INIT_ARRAY[%d] (%p)", i, init_array[i]);
                    void (*func)() = (void (*)())init_array[i];
                    func();
                } else {
                    Os::info("DT_INIT_ARRAY[%d] is empty", i);
                }
            }
            OS_DEBUG("Calling DT_INIT_ARRAY finished.");
        } else {
            Os::info("DT_INIT_ARRAY not found. (trouble ahead)");
        }

        if (!g_Test.passed()) {
            Os::fatal(
                "Tried to call constructors, but shared library constructor "
                "test, still fails.");
        }
        OS_DEBUG("Shared library constructor test passed, leaving WA code.");
    }
    //we will never use this mutex again.
    delete g_mutex; g_mutex = NULL;
}
#endif