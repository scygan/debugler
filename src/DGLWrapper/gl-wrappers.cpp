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

#include <cassert>
#include "pointers.h"
#include "actions.h"
#include "tls.h"
#include "debugger.h"


class DGLWrapperCookie {
public:

    DGLWrapperCookie(Entrypoint entrypoint):m_Call(entrypoint, 0), m_ThreadState(DGLThreadState::get()) {
        m_ProcessActions = m_ThreadState->enterActionProcessing();      
        if (m_ProcessActions) {
            tracePre();
        }
    }
#define VAR_CTOR(COUNT, TEMPL_ARG, FUNC_ARG, PARAMS)                                                                           \
    template<TEMPL_ARG>                                                                                                        \
    DGLWrapperCookie(Entrypoint entrypoint, FUNC_ARG):m_Call(entrypoint, COUNT), m_ThreadState(DGLThreadState::get()) {        \
        m_ProcessActions = m_ThreadState->enterActionProcessing();                                                             \
        if (m_ProcessActions) {                                                                                                \
            PARAMS ;                                                                                                           \
            tracePre();                                                                                                        \
        }                                                                                                                      \
    } 

#define TEMPL_ARG(n) typename T##n
#define FUNC_ARG(n) T##n arg##n
#define PARAMS(n)   m_Call << arg##n
#define DEFINITION ,
#define STATEMENT ;

#define EXPAND_1(EXPAND_ARG, JOIN) EXPAND_ARG(1)
#define EXPAND_2(EXPAND_ARG, JOIN) EXPAND_1(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(2)
#define EXPAND_3(EXPAND_ARG, JOIN) EXPAND_2(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(3)
#define EXPAND_4(EXPAND_ARG, JOIN) EXPAND_3(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(4)
#define EXPAND_5(EXPAND_ARG, JOIN) EXPAND_4(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(5)
#define EXPAND_6(EXPAND_ARG, JOIN) EXPAND_5(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(6)
#define EXPAND_7(EXPAND_ARG, JOIN) EXPAND_6(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(7)
#define EXPAND_8(EXPAND_ARG, JOIN) EXPAND_7(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(8)
#define EXPAND_9(EXPAND_ARG, JOIN) EXPAND_8(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(9)
#define EXPAND_10(EXPAND_ARG, JOIN) EXPAND_9(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(10)
#define EXPAND_11(EXPAND_ARG, JOIN) EXPAND_10(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(11)
#define EXPAND_12(EXPAND_ARG, JOIN) EXPAND_11(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(12)
#define EXPAND_13(EXPAND_ARG, JOIN) EXPAND_12(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(13)
#define EXPAND_14(EXPAND_ARG, JOIN) EXPAND_13(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(14)
#define EXPAND_15(EXPAND_ARG, JOIN) EXPAND_14(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(15)
#define EXPAND_16(EXPAND_ARG, JOIN) EXPAND_15(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(16)
#define EXPAND_17(EXPAND_ARG, JOIN) EXPAND_16(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(17)
#define EXPAND_18(EXPAND_ARG, JOIN) EXPAND_17(EXPAND_ARG, JOIN) JOIN EXPAND_ARG(18)



    VAR_CTOR(1, EXPAND_1(TEMPL_ARG, DEFINITION), EXPAND_1(FUNC_ARG, DEFINITION), EXPAND_1(PARAMS, STATEMENT))
    VAR_CTOR(2, EXPAND_2(TEMPL_ARG, DEFINITION), EXPAND_2(FUNC_ARG, DEFINITION), EXPAND_2(PARAMS, STATEMENT))
    VAR_CTOR(3, EXPAND_3(TEMPL_ARG, DEFINITION), EXPAND_3(FUNC_ARG, DEFINITION), EXPAND_3(PARAMS, STATEMENT))
    VAR_CTOR(4, EXPAND_4(TEMPL_ARG, DEFINITION), EXPAND_4(FUNC_ARG, DEFINITION), EXPAND_4(PARAMS, STATEMENT))
    VAR_CTOR(5, EXPAND_5(TEMPL_ARG, DEFINITION), EXPAND_5(FUNC_ARG, DEFINITION), EXPAND_5(PARAMS, STATEMENT))    
    VAR_CTOR(6, EXPAND_6(TEMPL_ARG, DEFINITION), EXPAND_6(FUNC_ARG, DEFINITION), EXPAND_6(PARAMS, STATEMENT))
    VAR_CTOR(7, EXPAND_7(TEMPL_ARG, DEFINITION), EXPAND_7(FUNC_ARG, DEFINITION), EXPAND_7(PARAMS, STATEMENT))
    VAR_CTOR(8, EXPAND_8(TEMPL_ARG, DEFINITION), EXPAND_8(FUNC_ARG, DEFINITION), EXPAND_8(PARAMS, STATEMENT))
    VAR_CTOR(9, EXPAND_9(TEMPL_ARG, DEFINITION), EXPAND_9(FUNC_ARG, DEFINITION), EXPAND_9(PARAMS, STATEMENT))
    VAR_CTOR(10, EXPAND_10(TEMPL_ARG, DEFINITION), EXPAND_10(FUNC_ARG, DEFINITION), EXPAND_10(PARAMS, STATEMENT))
    VAR_CTOR(11, EXPAND_11(TEMPL_ARG, DEFINITION), EXPAND_11(FUNC_ARG, DEFINITION), EXPAND_11(PARAMS, STATEMENT))
    VAR_CTOR(12, EXPAND_12(TEMPL_ARG, DEFINITION), EXPAND_12(FUNC_ARG, DEFINITION), EXPAND_12(PARAMS, STATEMENT))
    VAR_CTOR(13, EXPAND_13(TEMPL_ARG, DEFINITION), EXPAND_13(FUNC_ARG, DEFINITION), EXPAND_13(PARAMS, STATEMENT))
    VAR_CTOR(14, EXPAND_14(TEMPL_ARG, DEFINITION), EXPAND_14(FUNC_ARG, DEFINITION), EXPAND_14(PARAMS, STATEMENT))
    VAR_CTOR(15, EXPAND_15(TEMPL_ARG, DEFINITION), EXPAND_15(FUNC_ARG, DEFINITION), EXPAND_15(PARAMS, STATEMENT))
    VAR_CTOR(16, EXPAND_16(TEMPL_ARG, DEFINITION), EXPAND_16(FUNC_ARG, DEFINITION), EXPAND_16(PARAMS, STATEMENT))
    VAR_CTOR(17, EXPAND_17(TEMPL_ARG, DEFINITION), EXPAND_17(FUNC_ARG, DEFINITION), EXPAND_17(PARAMS, STATEMENT))
    VAR_CTOR(18, EXPAND_18(TEMPL_ARG, DEFINITION), EXPAND_18(FUNC_ARG, DEFINITION), EXPAND_18(PARAMS, STATEMENT))

    ~DGLWrapperCookie() {
        if (m_ProcessActions) {
            try {
                g_Actions[m_Call.getEntrypoint()]->Post(m_Call, retVal);
            } catch (const DGLDebugController::TeardownException&) {
                _g_Controller.reset();
                Os::terminate();
            } catch (const std::exception& e) {
                Os::fatal(e.what());
            }
            m_ThreadState->leaveActionProcessing();
        }
    }
    RetValue retVal;

private:

    void tracePre() {
        try {
            retVal = g_Actions[m_Call.getEntrypoint()]->Pre(m_Call);
        } catch (const DGLDebugController::TeardownException&) {
            _g_Controller.reset();
            Os::terminate();
        } catch (const std::exception& e) {
            Os::fatal(e.what());
        }
    }

    CalledEntryPoint m_Call;
   
    DGLThreadState* m_ThreadState;
    bool m_ProcessActions;
};


extern "C" {
#include "codegen/wrappers.inl"
}


#define FUNC_LIST_ELEM_SUPPORTED(name, type, library) (FUNC_PTR)&name##_Wrapper,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) NULL,
FUNC_PTR wrapperPtrs[] = {
    #include "codegen/functionList.inl"
    NULL
};
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED


FUNC_PTR getWrapperPointer(Entrypoint entryp) {
    return wrapperPtrs[entryp];
}

extern "C" {
#include "codegen/exporters.inl"
#ifndef __ANDROID__
    //on Linuxes ABI is not really expected and all EXT symbols are exported
    //It does not hurt Windows, also.
    #include "codegen/exporters-ext.inl"
#endif
}
