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

#define DGL_WRAPPERS_INSIDE
#include "gl-wrappers.h"

#include "globalstate.h"
#include "pointers.h"
#include "action-manager.h"
#include "tls.h"
#include "debugger.h"

#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
#include "wa-soctors.h"
#endif

class DGLWrapperCookie {
   public:
    DGLWrapperCookie(Entrypoint entrypoint)
            : m_Call(entrypoint, 0), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            tracePre();
        }
    }
    template <typename T1>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1)
            : m_Call(entrypoint, 1), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            tracePre();
        }
    }
    template <typename T1, typename T2>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2)
            : m_Call(entrypoint, 2), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3)
            : m_Call(entrypoint, 3), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
            : m_Call(entrypoint, 4), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5)
            : m_Call(entrypoint, 5), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6)
            : m_Call(entrypoint, 6), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7)
            : m_Call(entrypoint, 7), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8)
            : m_Call(entrypoint, 8), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
            : m_Call(entrypoint, 9), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
            : m_Call(entrypoint, 10), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11)
            : m_Call(entrypoint, 11), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12)
            : m_Call(entrypoint, 12), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13)
            : m_Call(entrypoint, 13), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13, typename T14>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13, T14 arg14)
            : m_Call(entrypoint, 14), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            m_Call.setArg(13, arg14);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13, typename T14,
              typename T15>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15)
            : m_Call(entrypoint, 15), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            m_Call.setArg(13, arg14);
            m_Call.setArg(14, arg15);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13, typename T14,
              typename T15, typename T16>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15,
                     T16 arg16)
            : m_Call(entrypoint, 16), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            m_Call.setArg(13, arg14);
            m_Call.setArg(14, arg15);
            m_Call.setArg(15, arg16);
            tracePre();
        }
    }
    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13, typename T14,
              typename T15, typename T16, typename T17>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15,
                     T16 arg16, T17 arg17)
            : m_Call(entrypoint, 17), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            m_Call.setArg(13, arg14);
            m_Call.setArg(14, arg15);
            m_Call.setArg(15, arg16);
            m_Call.setArg(16, arg17);
            tracePre();
        }
    }


    template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10,
              typename T11, typename T12, typename T13, typename T14,
              typename T15, typename T16, typename T17, typename T18>
    DGLWrapperCookie(Entrypoint entrypoint, T1 arg1, T2 arg2, T3 arg3, T4 arg4,
                     T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10,
                     T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15,
                     T16 arg16, T17 arg17, T18 arg18)
            : m_Call(entrypoint, 18), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();
        if (m_ProcessActions) {
            m_Call.setArg(0, arg1);
            m_Call.setArg(1, arg2);
            m_Call.setArg(2, arg3);
            m_Call.setArg(3, arg4);
            m_Call.setArg(4, arg5);
            m_Call.setArg(5, arg6);
            m_Call.setArg(6, arg7);
            m_Call.setArg(7, arg8);
            m_Call.setArg(8, arg9);
            m_Call.setArg(9, arg10);
            m_Call.setArg(10, arg11);
            m_Call.setArg(11, arg12);
            m_Call.setArg(12, arg13);
            m_Call.setArg(13, arg14);
            m_Call.setArg(14, arg15);
            m_Call.setArg(15, arg16);
            m_Call.setArg(16, arg17);
            m_Call.setArg(17, arg18);
            tracePre();
        }
    }


    ~DGLWrapperCookie() {
#ifdef DEBUG_WRAPPERS
        Os::info("tracePost %s", GetEntryPointName(m_Call.getEntrypoint()));
#endif
        if (m_ProcessActions) {
            try {
                GlobalState::getActionManager().GetAction(m_Call.getEntrypoint()).Post(m_Call, retVal);
            }
            catch (const DGLDebugController::TeardownException&) {
                GlobalState::reset();
                Os::terminate();
            }
            catch (const std::exception& e) {
                Os::fatal(e.what());
            }
            m_ThreadState->leaveActionProcessing();
        }
    }
    RetValue retVal;

   private:
    void tracePre() {
#ifdef DEBUG_WRAPPERS
        Os::info("tracePre %s", GetEntryPointName(m_Call.getEntrypoint()));
#endif
        try {
            retVal = GlobalState::getActionManager().GetAction(m_Call.getEntrypoint()).Pre(m_Call);
        }
        catch (const DGLDebugController::TeardownException&) {
            GlobalState::reset();
            Os::terminate();
        }
        catch (const std::exception& e) {
            Os::fatal(e.what());
        }
    }
#if DGL_HAVE_WA(ANDROID_SO_CONSTRUCTORS)
    DGLWASoCtors m_SoCtorsWa;
#endif

    CalledEntryPoint m_Call;

    DGLThreadState* m_ThreadState;
    bool m_ProcessActions;
};

namespace dgl_wrappers {
#include    "codegen_dgl_wrappers.inl"
}

#define FUNC_LIST_SUPPORTED_ELEM(name, type, library, retVal, params) \
    (dgl_func_ptr) & dgl_wrappers::name,
#define FUNC_LIST_NOT_SUPPORTED_ELEM(name, type, library, retVal, params) NULL,
dgl_func_ptr wrapperPtrs[] = {
#include "codegen_gl_function_list.inl"
        NULL};
#undef FUNC_LIST_SUPPORTED_ELEM
#undef FUNC_LIST_NOT_SUPPORTED_ELEM

dgl_func_ptr getWrapperPointer(Entrypoint entryp) { return wrapperPtrs[entryp]; }

extern "C" {

#ifdef HAVE_LIBRARY_WINGDI
//do not export wingdi symbols.
//We depend on this library, so it is impossible to get them "preloaded"
//We trace them using binary interception
#undef HAVE_LIBRARY_WINGDI 
#endif

#include "codegen_dgl_export.inl"
#ifdef __ANDROID__
// Export extension symbols exported by android system libraries.
// We don't want to  export all extensions due to same apps that
// use dlsym() to check if ext is supported...
#include "codegen_dgl_export_android.inl"
#else
// on Linuxes ABI is not really respected and all EXT symbols are exported
// It does not hurt Windows, also.
#include "codegen_dgl_export_ext.inl"
#endif
}
