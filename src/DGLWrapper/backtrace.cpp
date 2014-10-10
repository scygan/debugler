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

#include "backtrace.h"
#include "dl-intercept.h"
#include "dl.h"
#include "globalstate.h"

#include <DGLCommon/os.h>

#ifdef __ANDROID__
class BackTraceImpl: public BackTrace {
public:
    BackTraceImpl():m_Supported(false) {
        
        if (!s_CorkscrewDSO) {
            try {
                s_CorkscrewDSO = EarlyGlobalState::getDynLoader().getLibrary("libcorkscrew.so");
            } catch (const std::runtime_error& e) {
                Os::info(e.what());
            }
        }
        if (s_CorkscrewDSO && (!p_unwind_backtrace || !p_get_backtrace_symbols || !p_free_backtrace_symbols)) {
            p_unwind_backtrace = reinterpret_cast<t_unwind_backtrace>(s_CorkscrewDSO->getFunction("unwind_backtrace"));
            p_get_backtrace_symbols = reinterpret_cast<t_get_backtrace_symbols>(s_CorkscrewDSO->getFunction("get_backtrace_symbols"));
            p_free_backtrace_symbols = reinterpret_cast<t_free_backtrace_symbols>(s_CorkscrewDSO->getFunction("free_backtrace_symbols"));
        };

        m_Supported =  (p_unwind_backtrace && p_get_backtrace_symbols && p_free_backtrace_symbols);
    }  

   /*
   * Describes a single frame of a backtrace.
   */
    typedef struct {
        uintptr_t absolute_pc;     /* absolute PC offset */
        uintptr_t stack_top;       /* top of stack for this frame */
        size_t stack_size;         /* size of this stack frame */
    } backtrace_frame_t;

    /*
    * Describes the symbols associated with a backtrace frame.
    */
    typedef struct {
        uintptr_t relative_pc;       /* relative frame PC offset from the start of the library,
                                     or the absolute PC if the library is unknown */
        uintptr_t relative_symbol_addr; /* relative offset of the symbol from the start of the
                                        library or 0 if the library is unknown */
        char* map_name;              /* executable or library name, or NULL if unknown */
        char* symbol_name;           /* symbol name, or NULL if unknown */
        char* demangled_name;        /* demangled symbol name, or NULL if unknown */
    } backtrace_symbol_t;



    typedef ssize_t (* t_unwind_backtrace)(backtrace_frame_t* backtrace, size_t ignore_depth, size_t max_depth);
    typedef void (* t_get_backtrace_symbols)(const backtrace_frame_t* backtrace, size_t frames,
        backtrace_symbol_t* backtrace_symbols);
    typedef void (*t_format_backtrace_line)(unsigned frameNumber, const backtrace_frame_t* frame,
        const backtrace_symbol_t* symbol, char* buffer, size_t bufferSize);
    
    typedef void (* t_free_backtrace_symbols)(backtrace_symbol_t* backtrace_symbols, size_t frames);

private:
    enum {
        MAX_DEPTH = 31,
        MAX_BACKTRACE_LINE_LENGTH = 800
    };

    virtual void streamTo(std::vector<std::string>& str) override {

        if (!m_Supported) {
            throw std::runtime_error("Backtrace support not implemented on this platform.");
        }

        backtrace_frame_t frames[MAX_DEPTH];

        const int ignoreDepth = 1;

        ssize_t count = p_unwind_backtrace(frames, ignoreDepth + 1, MAX_DEPTH);

        void* currentLibraryAddress = DynamicLoader::getCurrentLibraryBaseAddress();

        if (count > 0) {
            std::vector<backtrace_symbol_t> symbols(count);
            p_get_backtrace_symbols(frames, count, &symbols[0]);
            for (ssize_t i = 0; i < count; i++) {

                Dl_info symbolInfo; 
                dladdr(reinterpret_cast<void*>(frames[i].absolute_pc), &symbolInfo);
                Os::info("XXX %x == %x\n", symbolInfo.dli_fbase,  currentLibraryAddress);
                if (symbolInfo.dli_fbase == currentLibraryAddress) {
                    //we assume, that removing current library symbols from backtrace
                    //is enough, to filter out DGL from backtrace. This implies no 3rd party
                    //libraries should be called from exporter to get here.
                    continue;
                }


                std::stringstream line;

                if (!symbols[i].map_name) {
                    line << "<unknown module>";
                } else {
                    line << symbols[i].map_name;
                }
                line << '!';
                line << "0x" << std::hex << (int) frames[i].absolute_pc << std::dec << ' ';

                if (symbols[i].demangled_name) {
                    line << symbols[i].demangled_name;
                } else if (symbols[i].symbol_name) {
                    line << symbols[i].symbol_name;
                } else {
                    line << "<unknown function>";
                }

                uint32_t pc_offset = symbols[i].relative_pc - symbols[i].relative_symbol_addr;

                if (pc_offset) {
                    line << " +" << pc_offset;
                }

                str.push_back(line.str());
            }
            p_free_backtrace_symbols(&symbols[0], count);
        }
     }

    bool m_Supported;

    static DynamicLibrary* s_CorkscrewDSO;
    static t_unwind_backtrace p_unwind_backtrace;
    static t_get_backtrace_symbols p_get_backtrace_symbols;
    static t_free_backtrace_symbols p_free_backtrace_symbols;

};
DynamicLibrary* BackTraceImpl::s_CorkscrewDSO = nullptr;
BackTraceImpl::t_unwind_backtrace BackTraceImpl::p_unwind_backtrace = nullptr;
BackTraceImpl::t_get_backtrace_symbols BackTraceImpl::p_get_backtrace_symbols = nullptr;
BackTraceImpl::t_free_backtrace_symbols BackTraceImpl::p_free_backtrace_symbols = nullptr;

#else
#ifdef _WIN32

#include "external/StackWalker/StackWalker.h"
#include "Psapi.h"
#include <sstream>
#include <boost/noncopyable.hpp>

class BackTraceImpl: public BackTrace {

    class DGLStackWalker: public StackWalker, boost::noncopyable {
    public:
        DGLStackWalker(std::vector<std::string>& buffer): m_buffer(buffer) {
            MODULEINFO modInfo;
            if (GetModuleInformation(GetCurrentProcess(), (HMODULE)Os::getCurrentModuleHandle(), &modInfo, sizeof(modInfo))) {
                m_BoundsOfDGLLibrary[0] = reinterpret_cast<intptr_t>(modInfo.lpBaseOfDll);
                m_BoundsOfDGLLibrary[1] = reinterpret_cast<intptr_t>(reinterpret_cast<char*>(modInfo.lpBaseOfDll) + modInfo.SizeOfImage);
            }
        }

        virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry) {

            //we assume, that removing current library symbols from backtrace
            //is enough, to filter out DGL from backtrace. This implies no 3rd party
            //libraries should be called from exporter to get here.
            bool outSideOfCurrentLibrary = ((intptr_t)entry.offset <  m_BoundsOfDGLLibrary[0] ||
                                            (intptr_t)entry.offset >= m_BoundsOfDGLLibrary[1]);

            if ( (eType != lastEntry) && (entry.offset != 0) && outSideOfCurrentLibrary)
            {
                std::stringstream line;

                if (entry.moduleName[0] == 0 && entry.loadedImageName == 0) {
                    line << "<unknown module>";
                } else {
                    const char* moduleName;
                    if (!entry.moduleName) {
                        //use full filepath
                        moduleName = entry.loadedImageName;
                    } else {
                        //use module name
                        moduleName = entry.moduleName;

                        if (entry.loadedImageName) {
                            //if we have both, use full file path to obtain file ext.
                            const char* offset = strstr(entry.loadedImageName, entry.moduleName);
                            while (offset) {
                                moduleName = offset;
                                offset = strstr(offset + 1, entry.moduleName);
                            }
                        }
                    }
                    line << moduleName;
                }
                line << '!';
                line << "0x" << std::hex << (int) entry.offset << std::dec << ' ';

                if (entry.undFullName[0] != 0) {
                    line << entry.undFullName;
                } else if (entry.undName[0] != 0) {
                    line << entry.undName;
                } else  if (entry.name[0] != 0) {
                    line << entry.name;
                } else {
                    line << "<unknown function>";
                }
                line << ' ';

                if (entry.lineFileName[0] == 0) {
                    line << "<unknown source file>";
                } else {
                    line << entry.lineFileName;
                    if (entry.lineNumber) {
                        line << " Line " << entry.lineNumber;
                    }
                }
                m_buffer.push_back(line.str());
            }
            StackWalker::OnCallstackEntry(eType, entry);
        }
        std::vector<std::string>& m_buffer;
        intptr_t m_BoundsOfDGLLibrary[2];
    };

    virtual void streamTo(std::vector<std::string>& str) override {
        DGLStackWalker sw(str);
        sw.ShowCallstack(); 
    }
};

#else
#ifdef __linux__

#include <execinfo.h>

class BackTraceImpl: public BackTrace {
    virtual void streamTo(std::vector<std::string>& ret) override {
        ret.resize(20);
        std::vector<void*> buffer;
        int nptrs = backtrace(&buffer[0], buffer.size());
        while (nptrs > 0 && nptrs == static_cast<int>(buffer.size())) {
            buffer.resize(buffer.size() * 2);
            nptrs = backtrace(&buffer[0], buffer.size());
        }
        char** strings = backtrace_symbols(&buffer[0], buffer.size());
        if (!strings) {
            throw std::runtime_error("Cannot get backtrace: backtrace_symbols failed.");
        }
        ret.resize(buffer.size());
        for (size_t i = 0; i < ret.size(); i++) {
            ret[i] = strings[i];
        }
        free(strings);
    }
};

#else
class BackTraceImpl: public BackTrace {
    virtual void streamTo(std::vector<std::string>&) override {
        throw std::runtime_error("Backtrace support not implemented on this platform.");
    }
};
#endif
#endif
#endif

std::shared_ptr<BackTrace> BackTrace::Get() {
    return std::make_shared<BackTraceImpl>();
}
