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


//#include <boost/make_shared.hpp>

#include "gl-wrappers.h"
#include "debugger.h"
#include "actions.h"
#include "pointers.h"
#include "api-loader.h"
#include "tls.h"
#include "display.h"
#include "native-surface.h"

#include <DGLNet/server.h>
#include <DGLCommon/gl-types.h>

boost::shared_ptr<ActionBase> g_Actions[NUM_ENTRYPOINTS];

THREAD_LOCAL int ActionBase::m_ThreadedInfiniteRecursionGuard = 0;

RetValue ActionBase::DoPre(const CalledEntryPoint& call) {
    
    m_ThreadedInfiniteRecursionGuard++;

    if (m_ThreadedInfiniteRecursionGuard > 1) {
        //	This is unlikely, but may happen sometimes - OpenGL implementation called us. 
        //If we dont catch it here, we will deadlock later, or likely get into infinite recursion.
        return RetValue();
    } else {
        try {
            return Pre(call);
        } catch (const DGLDebugController::TeardownException&) {
            _g_Controller.reset();
            Os::terminate();
        } catch (const std::exception& e) {
            Os::fatal(e.what());
        }
    }
    return RetValue();
}

void ActionBase::DoPost(const CalledEntryPoint& call, const RetValue& ret) {

    if (m_ThreadedInfiniteRecursionGuard == 1) {
        try {
            Post(call, ret);
        } catch (const DGLDebugController::TeardownException&) {
            _g_Controller.reset();
            Os::terminate();
        } catch (const std::exception& e) {
            Os::fatal(e.what());
        }
    }

    m_ThreadedInfiniteRecursionGuard--;
}


void ActionBase::SetPrev(const boost::shared_ptr<ActionBase>& prev) {
    m_PrevAction = prev;
}

RetValue ActionBase::Pre(const CalledEntryPoint& call) {
    return PrevPre(call);
}

void ActionBase::Post(const CalledEntryPoint& call, const RetValue& ret) {
    return PrevPost(call, ret);
}


RetValue ActionBase::PrevPre(const CalledEntryPoint& call) {
    if (m_PrevAction)
        return m_PrevAction->Pre(call);
    return RetValue();
}

void ActionBase::PrevPost(const CalledEntryPoint& call, const RetValue& ret) {
    if (m_PrevAction)
        m_PrevAction->Post(call, ret);
}

RetValue DefaultAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    std::lock_guard<std::mutex> server_lock(getController()->getServer().getMtx());

    //do a fast non-blocking poll to get "interrupt" message, etc.."
    getController()->poll();

    //check if any break is pending
    if (getController()->getBreakState().mayBreakAt(call.getEntrypoint())) {
        //we just hit a break;
        dglState::GLContext* ctx = gc;
        dglnet::message::BreakedCall callStateMessage(call, (value_t)getController()->getCallHistory().size(), ctx?ctx->getId():0, DGLDisplayState::describeAll());
        getController()->getServer().sendMessage(&callStateMessage);
    }
    
    while (getController()->getBreakState().isBreaked()) {
        //iterate block & loop until someone unbreaks us
        getController()->run_one();
    }

    //now there should be no breaks

    //add call to history ring
    getController()->getCallHistory().add(call);

    return ret;
}

void DefaultAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    std::lock_guard<std::mutex> server_lock(getController()->getServer().getMtx());

    CallHistory& history = getController()->getCallHistory();

    GLenum error;
    if (dglState::GLContext* ctx = gc) {

        bool hasDebugOutput = ctx->hasDebugOutput();
        if (hasDebugOutput) {
            history.setDebugOutput(ctx->popDebugOutput());
            getController()->getBreakState().setBreakAtDebugOutput();
        }

        if ((error = gc->peekError()) != GL_NO_ERROR) {
            history.setError(error);
            getController()->getBreakState().setBreakAtGLError(error);
        }
    }

    if (ret.isSet()) {
        history.setRetVal(ret);    
    }
    
    PrevPost(call, ret);
}



RetValue GLGetErrorAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);
    
    if (ret.isSet()) return ret;

    if (gc && call.getEntrypoint() == glGetError_Call) {
        std::pair<bool, GLenum> pokedError = gc->getPokedError();
        if (pokedError.first) {
            ret = pokedError.second;
        }
    }

    return ret;
}

RetValue GetProcAddressAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    //get entrypoint name
    const char* entrpName; call.getArgs()[0].get(entrpName);

    //translate entrypoint to debugger enum
    Entrypoint entryp = GetEntryPointEnum(entrpName);
    if (entryp == NO_ENTRYPOINT) {
        //entrypoint not supported by debugger. retult of native *GetProcAddress
        //will be passed to application, making a hole in trace.

        //TODO: add partial support for unknown entrypoints
        return ret;
    }
    //Load and get address of entrypoint implementation
    if (g_ApiLoader.loadExtPointer(entryp)) {
        //entrypoint supported by implementation, return address of wrapper to application
        ret = reinterpret_cast<FUNC_PTR>(getWrapperPointer(entryp));
    } else {
        //entrypoint unsupported by implementation, return NULL to application
        ret = (FUNC_PTR) NULL;
    }
    return ret;
}

#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
void SurfaceAction::Post(const CalledEntryPoint& call, const RetValue& ret) {

    EGLSurface surface;
    ret.get(surface);
    if (surface != EGL_NO_SURFACE) {
        EGLDisplay dpy;
        call.getArgs()[0].get(dpy);
        EGLConfig config;     
        call.getArgs()[1].get(config);
        
        DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy))->addSurface<dglState::NativeSurfaceEGL>(
            reinterpret_cast<opaque_id_t>(surface), reinterpret_cast<opaque_id_t>(config));
    }
}
#endif

void ContextAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
#ifdef HAVE_LIBRARY_WGL
    HGLRC ctx;
    HDC device;
    BOOL retBool;
#endif    
#ifdef HAVE_LIBRARY_GLX
    GLXContext ctx;
    GLXDrawable drawable;
    Display* dpy;
    Bool retBool;
#endif
    EGLBoolean eglBool;
    EGLSurface eglReadSurface;
    EGLContext eglCtx;
    EGLDisplay eglDpy;
    GLenumWrap enumWrapped;

    Entrypoint entryp = call.getEntrypoint();

    switch (entryp) {
#ifdef HAVE_LIBRARY_WGL
        case wglCreateContext_Call:
        case wglCreateContextAttribsARB_Call:
            ret.get(ctx);
            if (NULL != ctx) {
                DGLDisplayState::defDpy()->ensureContext(dglState::GLContextVersion::DT, reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case wglMakeCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(device);
                call.getArgs()[1].get(ctx);

                dglState::NativeSurfaceBase* surface = NULL;
                if (device) {
                    surface = DGLDisplayState::defDpy()->ensureSurface<dglState::NativeSurfaceWGL>
                        ((opaque_id_t)device)->second.get();
                }

                DGLThreadState::get()->bindContext(DGLDisplayState::defDpy(), 
                    reinterpret_cast<opaque_id_t>(ctx), surface);
            }
            break;
        case wglDeleteContext_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(ctx);
                DGLDisplayState::defDpy()->deleteContext(reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
#endif
#ifdef HAVE_LIBRARY_GLX
        case glXCreateContextAttribsARB_Call:
        case glXCreateContext_Call:
        case glXCreateNewContext_Call:
            ret.get(ctx);
            if (ctx) {
                call.getArgs()[0].get(dpy);
                DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy))->ensureContext(
                        dglState::GLContextVersion::DT, reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case glXMakeCurrent_Call:
        case glXMakeContextCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(dpy);

                if (entryp == glXMakeCurrent_Call) {

                    call.getArgs()[1].get(drawable);
                    call.getArgs()[2].get(ctx);

                } else if (entryp == glXMakeContextCurrent_Call) {

                    call.getArgs()[2].get(drawable); //read drawable
                    call.getArgs()[3].get(ctx);
                }
                                
                dglState::NativeSurfaceBase* surface = NULL;
                if (drawable) {
                    surface = DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy))->ensureSurface<dglState::NativeSurfaceGLX>
                        ((opaque_id_t)drawable)->second.get();
                }
                DGLThreadState::get()->bindContext(DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy)), 
                    reinterpret_cast<opaque_id_t>(ctx), surface);
            }
            break;
        case glXDestroyContext_Call:
            call.getArgs()[0].get(dpy);
            call.getArgs()[0].get(ctx);
            DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy))->lazyDeleteContext(reinterpret_cast<opaque_id_t>(ctx));
            break;
#endif
        case eglBindAPI_Call:
            ret.get(eglBool);
            if (eglBool) {
                call.getArgs()[0].get(enumWrapped);
                DGLThreadState::get()->bindEGLApi(enumWrapped);
            }
            break;
        case eglCreateContext_Call:
            ret.get(eglCtx);
            if (NULL != eglCtx) {
                call.getArgs()[0].get(eglDpy);
                if (DGLThreadState::get()->getEGLApi() == EGL_OPENGL_ES_API) {
                    EGLint const* attribList;
                    call.getArgs()[3].get(attribList);

                    ApiLibrary lib = LIBRARY_ES2;

                    if (attribList) {
                        for (int i = 0; attribList[i] != EGL_NONE; i += 2) {
                            if (attribList[i] == EGL_CONTEXT_CLIENT_VERSION) {
                                switch (attribList[i + 1]) {
                                    case 2: 
                                        lib = LIBRARY_ES2;
                                        break;
                                    case 3:
                                        lib = LIBRARY_ES3;
                                        break;
                                    default:
                                        assert(0);
                                        return;
                                }
                            }
                        }
                    }

                    g_ApiLoader.loadLibrary(lib);

                    DGLDisplayState::get((opaque_id_t)eglDpy)->ensureContext(
                        dglState::GLContextVersion::ES, reinterpret_cast<opaque_id_t>(eglCtx));

                } else if (DGLThreadState::get()->getEGLApi() == EGL_OPENGL_API) {
                    g_ApiLoader.loadLibrary(LIBRARY_GL);

                    DGLDisplayState::get((opaque_id_t)eglDpy)->ensureContext(
                        dglState::GLContextVersion::DT, reinterpret_cast<opaque_id_t>(eglCtx));
                }
                
            }
            break;
        case eglMakeCurrent_Call:
            ret.get(eglBool);
            if (eglBool) {
                call.getArgs()[0].get(eglDpy);
                call.getArgs()[2].get(eglReadSurface);
                call.getArgs()[3].get(eglCtx);

                dglState::NativeSurfaceBase* surface = NULL;
                if (eglReadSurface) {
#ifdef WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID
                    surface = DGLDisplayState::get((opaque_id_t)eglDpy)->getSurface((opaque_id_t)eglReadSurface)->second.get();
#else
                    surface = DGLDisplayState::get((opaque_id_t)eglDpy)->ensureSurface((opaque_id_t)eglReadSurface)->second.get();
#endif
                }
                DGLThreadState::get()->bindContext(
                    DGLDisplayState::get((opaque_id_t)eglDpy), 
                    reinterpret_cast<opaque_id_t>(eglCtx), surface);
            }
            break;
        case eglDestroyContext_Call:
            call.getArgs()[0].get(eglDpy);
            ret.get(eglBool);
            if (eglBool) {
                call.getArgs()[1].get(eglCtx);
                DGLDisplayState::get((opaque_id_t)eglDpy)->lazyDeleteContext(reinterpret_cast<opaque_id_t>(eglCtx));
            }
            break;
        case eglReleaseThread_Call:
            ret.get(eglBool);
            if (eglBool) {
                DGLThreadState::release();
            }
            break;
    }
    PrevPost(call, ret);
}

bool DebugContextAction::anyContextPresent = false;

RetValue DebugContextAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    if (!g_Config.m_ForceDebugContext) {
        return ret;
    }

#ifdef HAVE_LIBRARY_WGL
    HDC hdc = NULL;
    HGLRC sharedCtx = NULL;
    const int *attribList = NULL;
    switch (call.getEntrypoint()) {
        case wglCreateContext_Call:
            call.getArgs()[0].get(hdc);
            break;
        case wglCreateContextAttribsARB_Call:
            call.getArgs()[0].get(hdc);
            call.getArgs()[1].get(sharedCtx);
            call.getArgs()[2].get(attribList);
            break;
        default:
            assert(0);
    }

    std::vector<int> newAttribList;
    bool done = false;
    if (attribList != NULL) {
        int i = 0;
        while (attribList[i]) {
            int attrib = attribList[i++], value = attribList[i++]; 
            if (attrib == WGL_CONTEXT_FLAGS_ARB) {
                if (!g_Config.m_ForceDebugContextES && (value & WGL_CONTEXT_ES2_PROFILE_BIT_EXT)) {
                    return ret;
                }
                value |= WGL_CONTEXT_DEBUG_BIT_ARB;
                done = true;
            }
            newAttribList.push_back(attrib);
            newAttribList.push_back(value);
        }
    }
    newAttribList.push_back(0);
    if (!done) {
        newAttribList[newAttribList.size() - 1] = WGL_CONTEXT_FLAGS_ARB;
        newAttribList.push_back(WGL_CONTEXT_DEBUG_BIT_ARB);
        newAttribList.push_back(0);
    }

    HGLRC tmpCtx = NULL;

    if (!anyContextPresent) {
        //we must create one dummy ctx, to force ICD loading on Windows
        //otherwise wglCreateContextAttribsARB, which is an extension, will not be availiable
        tmpCtx = DIRECT_CALL_CHK(wglCreateContext)(hdc);
        DIRECT_CALL_CHK(wglMakeCurrent)(hdc, tmpCtx);
    }

    //call wglCreateContextAttribsARB only if supported by implementation. Otherwise do nothing - ctx will be created in wrapper function
    if (POINTER(wglCreateContextAttribsARB) || g_ApiLoader.loadExtPointer(wglCreateContextAttribsARB_Call)) {
        ret = DIRECT_CALL_CHK(wglCreateContextAttribsARB)(hdc, sharedCtx, &newAttribList[0]);
    }

    if (tmpCtx) {
        //unwind dummy ctx
        DIRECT_CALL_CHK(wglMakeCurrent)(NULL, NULL);
        DIRECT_CALL_CHK(wglDeleteContext)(tmpCtx);
        anyContextPresent = true;
    }
#else
#pragma warning "DebugContextAction::Pre not implemented"
#endif
    return ret;
}

void TextureAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenTextures_Call || entrp == glGenTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ensureTexture(names[i]);
            }
        } else if (entrp == glDeleteTextures_Call || entrp == glDeleteTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->deleteTexture(names[i]);
            }
        } else if (entrp == glBindTexture_Call || entrp == glBindTextureEXT_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            gc->ensureTexture(name)->setTarget(target);
        }
    }
    PrevPost(call, ret);
}

void BufferAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenBuffers_Call || entrp ==  glGenBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ensureBuffer(names[i]);
            }
        } else if (entrp == glDeleteBuffers_Call || entrp == glDeleteBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->deleteBuffer(names[i]);
            }
        } else if (entrp == glBindBuffer_Call || entrp == glBindBufferARB_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                gc->ensureBuffer(name)->setTarget(target);
            }
        }
    }
    PrevPost(call, ret);
}

void ProgramAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (gc) {
        GLuint name;
        if (entrp == glCreateProgram_Call || entrp == glCreateProgramObjectARB_Call) {

            ret.get(name);

            gc->ensureProgram(name, entrp == glCreateProgramObjectARB_Call);

        } else if (entrp == glDeleteProgram_Call || entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLProgramObj* program = gc->ensureProgram(name, entrp == glDeleteObjectARB_Call);
            program->markDeleted();
            if (program->mayDelete()) {
                gc->deleteProgram(name);
            }

        } else if (entrp == glUseProgram_Call || entrp == glUseProgramObjectARB_Call) {

            call.getArgs()[0].get(name);

            GLuint currentProgramName;
            {
                GLint i;
                DIRECT_CALL(glGetIntegerv)(GL_CURRENT_PROGRAM, &i);
                currentProgramName = static_cast<GLuint>(i);
            }
            

            if (currentProgramName != name) {

                //we may delete last program, if marked for deletion

                dglState::GLProgramObj* currentProgram = gc->ensureProgram(currentProgramName, entrp == glUseProgramObjectARB_Call);

                currentProgram->use(false);
                if (currentProgram->mayDelete()) {
                    gc->deleteProgram(currentProgramName);
                }
            }

            if (name != 0) {
                gc->ensureProgram(name, entrp == glUseProgramObjectARB_Call)->use(true);
            }
        } else if (entrp == glLinkProgram_Call) {

            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetProgramiv)(name, GL_LINK_STATUS, &linkStatus);

            if (linkStatus != GL_TRUE) {
                getController()->getBreakState().setBreakAtCompilerError();
            }

        } else if (entrp == glLinkProgramARB_Call) {
            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetObjectParameterivARB)(name, GL_OBJECT_LINK_STATUS_ARB, &linkStatus);

            if (linkStatus != GL_TRUE) {
                getController()->getBreakState().setBreakAtCompilerError();
            }

        }
    }
    PrevPost(call, ret);
}

void ShaderAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (gc) {
        GLuint name;
        if (entrp == glCreateShader_Call || entrp == glCreateShaderObjectARB_Call) {

            //we assume that GLhandleARB is the same type as GLuint

            ret.get(name);

            GLenumWrap target;
            call.getArgs()[0].get(target);

            gc->ensureShader(name, entrp == glCreateShaderObjectARB_Call)->createCalled(target);

        } else if (entrp == glDeleteShader_Call || entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLShaderObj* shader = gc->ensureShader(name, entrp == glDeleteObjectARB_Call);

            shader->deleteCalled();

        } else if (entrp == glCompileShader_Call || entrp == glCompileShaderARB_Call) {
            
            call.getArgs()[0].get(name);

            dglState::GLShaderObj* shader = gc->ensureShader(name, entrp == glCompileShaderARB_Call);
            GLint compileStatus = shader->queryCompilationStatus();
            
            if (compileStatus != GL_TRUE) {
                getController()->getBreakState().setBreakAtCompilerError();
            }

        } else if (entrp == glAttachShader_Call || entrp == glAttachObjectARB_Call) {

            GLuint prog, shad;
            call.getArgs()[0].get(prog);
            call.getArgs()[1].get(shad);
            gc->ensureProgram(prog, entrp == glAttachObjectARB_Call)->attachShader(gc->ensureShader(shad, entrp == glAttachObjectARB_Call));

        } else if (entrp == glDetachShader_Call || entrp == glDetachObjectARB_Call) {

            GLuint prog, shad;
            call.getArgs()[0].get(prog);
            call.getArgs()[1].get(shad);
            gc->ensureProgram(prog, entrp == glDetachObjectARB_Call)->detachShader(gc->ensureShader(shad, entrp == glAttachObjectARB_Call));

        } else if (entrp == glShaderSourceARB_Call || entrp == glShaderSource_Call) {

            GLuint shad;
            call.getArgs()[0].get(shad);
            gc->ensureShader(shad, entrp == glShaderSourceARB_Call)->shaderSourceCalled();

        }
    }
    PrevPost(call, ret);
}

void ImmediateModeAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    if (gc) {
        switch (call.getEntrypoint()) {
            case glBegin_Call:
                gc->setImmediateMode(true);
                break;
            case glEnd_Call:
                gc->setImmediateMode(false);
                break;
        }
    }
    PrevPost(call, ret);
}


void FBOAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenFramebuffers_Call || entrp == glGenFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ensureFBO(names[i]);
            }
        } else if (entrp == glDeleteFramebuffers_Call || entrp == glDeleteFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->deleteFBO(names[i]);
            }
        } else if (entrp == glBindFramebuffer_Call || entrp == glBindFramebufferEXT_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                gc->ensureFBO(name)->setTarget(target);
            }
        }
    }
    PrevPost(call, ret);
}

RetValue DebugOutputCallback::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    Entrypoint entrp = call.getEntrypoint();
    if (gc && (entrp == glDebugMessageCallback_Call || entrp == glDebugMessageCallbackARB_Call)) {
        GLDEBUGPROC callback;
        const GLvoid* userParam;
        call.getArgs()[0].get(callback);
        call.getArgs()[1].get(userParam);

        //register application's callback into out context
        gc->setCustomDebugOutputCallback(callback);

        //call debug message callback now, to prevent override
        if (entrp == glDebugMessageCallback_Call) {
            DIRECT_CALL_CHK(glDebugMessageCallback)(dglState::GLContext::debugOutputCallback, userParam);
        } else if (entrp == glDebugMessageCallbackARB_Call) {
            DIRECT_CALL_CHK(glDebugMessageCallbackARB)(dglState::GLContext::debugOutputCallback, userParam);
        } else { assert(0); }

        return RetValue::getVoidAlreadySet();
    }
    return ret; 
}
