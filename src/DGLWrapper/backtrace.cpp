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

#ifdef __ANDROID__
class BackTraceImpl: public BackTrace {
public:
    BackTraceImpl():m_Supported(false) {
        
        if (!s_CorkscrewDSO) {
            s_CorkscrewDSO = dlopen("libcorkscrew.so", RTLD_NOW);
        }
        if (s_CorkscrewDSO && (!p_unwind_backtrace || !p_get_backtrace_symbols || !p_format_backtrace_line || !p_free_backtrace_symbols)) {
            p_unwind_backtrace = reinterpret_cast<t_unwind_backtrace>(
                reinterpret_cast<ptrdiff_t>(dlsym(s_CorkscrewDSO, "unwind_backtrace")));
            p_get_backtrace_symbols = reinterpret_cast<t_get_backtrace_symbols>(
                reinterpret_cast<ptrdiff_t>(dlsym(s_CorkscrewDSO, "get_backtrace_symbols")));
            p_format_backtrace_line = reinterpret_cast<t_format_backtrace_line>(
                reinterpret_cast<ptrdiff_t>(dlsym(s_CorkscrewDSO, "format_backtrace_line")));
            p_free_backtrace_symbols = reinterpret_cast<t_free_backtrace_symbols>(
                reinterpret_cast<ptrdiff_t>(dlsym(s_CorkscrewDSO, "free_backtrace_symbols")));
        };

        m_Supported =  (p_unwind_backtrace && p_get_backtrace_symbols && p_format_backtrace_line && p_free_backtrace_symbols);
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

    virtual std::string str() override {

        if (!m_Supported) {
            return "Backtrace support not implemented on this platform.";
        }

        backtrace_frame_t stack[MAX_DEPTH];

        const int ignoreDepth = 1;

        ssize_t count = p_unwind_backtrace(stack, ignoreDepth + 1, MAX_DEPTH);

        std::ostringstream backTraceStream;

        if (count > 0) {
            std::vector<backtrace_symbol_t> symbols(count);
            p_get_backtrace_symbols(stack, count, &symbols[0]);
            for (ssize_t i = 0; i < count; i++) {
                char line[MAX_BACKTRACE_LINE_LENGTH];
                p_format_backtrace_line(i, &stack[i], &symbols[i],
                    line, MAX_BACKTRACE_LINE_LENGTH);
                backTraceStream << line << std::endl;
            }
            p_free_backtrace_symbols(&symbols[0], count);
        }

        return backTraceStream.str();
    }

    bool m_Supported;

    static void* s_CorkscrewDSO;
    static t_unwind_backtrace p_unwind_backtrace;
    static t_get_backtrace_symbols p_get_backtrace_symbols;
    static t_format_backtrace_line p_format_backtrace_line;
    static t_free_backtrace_symbols p_free_backtrace_symbols;

};
void* BackTraceImpl::s_CorkscrewDSO = nullptr;
BackTraceImpl::t_unwind_backtrace BackTraceImpl::p_unwind_backtrace = nullptr;
BackTraceImpl::t_get_backtrace_symbols BackTraceImpl::p_get_backtrace_symbols = nullptr;
BackTraceImpl::t_format_backtrace_line BackTraceImpl::p_format_backtrace_line = nullptr;
BackTraceImpl::t_free_backtrace_symbols BackTraceImpl::p_free_backtrace_symbols = nullptr;

#else
class BackTraceImpl: public BackTrace {
    virtual std::string str() override {
        return "Backtrace support not implemented on this platform.";
    }
};
#endif

std::shared_ptr<BackTrace> BackTrace::Get() {
    return std::make_shared<BackTraceImpl>();
}
