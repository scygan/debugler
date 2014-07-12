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

#include "actions.h"

#include "gl-wrappers.h"
#include "debugger.h"
#include "pointers.h"
#include "api-loader.h"
#include "tls.h"
#include "display.h"
#include "native-surface.h"
#include "gl-utils.h"
#include "globalstate.h"

#include <DGLCommon/gl-types.h>

#include <DGLNet/protocol/message.h>

#include "action-manager.h"

namespace actions {

void ActionBase::SetPrev(const std::shared_ptr<ActionBase>& prev) {
    m_PrevAction = prev;
}

RetValue ActionBase::Pre(const CalledEntryPoint& call) { return PrevPre(call); }

void ActionBase::Post(const CalledEntryPoint& call, const RetValue& ret) {
    return PrevPost(call, ret);
}

RetValue ActionBase::PrevPre(const CalledEntryPoint& call) {
    if (m_PrevAction) return m_PrevAction->Pre(call);
    return RetValue();
}

void ActionBase::PrevPost(const CalledEntryPoint& call, const RetValue& ret) {
    if (m_PrevAction) m_PrevAction->Post(call, ret);
}

RetValue DefaultAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    bool newConnection;

    DGLDebugController& controller =  GlobalState::getDebugController();

    do {

        newConnection = false;

        std::lock_guard<std::mutex> server_lock(
            controller.getServer().getMutex());

        // do a fast non-blocking poll to get "interrupt" message, etc.."
        controller.poll();

        // check if any break is pending
        if (controller.getBreakState().mayBreakAt(call.getEntrypoint())) {
            // we just hit a break;
            dglState::GLContext* ctx = gc;
            dglnet::message::BreakedCall callStateMessage(
                call, (value_t)controller.getCallHistory().size(),
                ctx ? ctx->getId() : 0, DGLDisplayState::describeAll());
           controller.getServer().getTransport()->sendMessage(
                &callStateMessage);
        }

        while (controller.getBreakState().isBreaked()) {
            // iterate block & loop until someone unbreaks us

            controller.run_one(newConnection);

            if (newConnection) {
                //in the meantime reconnection happened.

                //In such case start processing this part of Pre() once again.
                
                break;
            }
        }

    } while (newConnection);

    // now there should be no breaks

    // add call to history ring
    controller.getCallHistory().add(call);

    return ret;
}

void DefaultAction::Post(const CalledEntryPoint& call, const RetValue& ret) {

    DGLDebugController& controller =  GlobalState::getDebugController();

    std::lock_guard<std::mutex> server_lock(
            controller.getServer().getMutex());

    CallHistory& history = controller.getCallHistory();

    GLenum error;
    if (dglState::GLContext* ctx = gc) {

        bool hasDebugOutput = ctx->hasDebugOutput();
        if (hasDebugOutput) {
            history.setDebugOutput(ctx->popDebugOutput());
            controller.getBreakState().setBreakAtDebugOutput();
        }

        if ((error = gc->peekError()) != GL_NO_ERROR) {
            history.setError(error);
            controller.getBreakState().setBreakAtGLError(error);
        }
    }

    if (ret.isSet()) {
        history.setRetVal(ret);
    }

    PrevPost(call, ret);
}

void GLGetErrorAction::Register(ActionManager& manager) {
    std::shared_ptr<GLGetErrorAction> obj
        = std::make_shared<GLGetErrorAction>();
    manager.RegisterAction(glGetError_Call, obj);
}


void ErrorAwareGLAction::Post(const CalledEntryPoint& call, const RetValue& ret) {

    if (gc && gc->peekError() == GL_NO_ERROR) {
        NoGLErrorPost(call, ret);
    } else {
        PrevPost(call, ret);
    }
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

void GetProcAddressAction::Register(ActionManager& manager) {
    std::shared_ptr<GetProcAddressAction> obj
        = std::make_shared<GetProcAddressAction>();
    manager.RegisterAction(wglGetProcAddress_Call, obj);
    manager.RegisterAction(glXGetProcAddress_Call, obj);
    manager.RegisterAction(glXGetProcAddressARB_Call, obj);
    manager.RegisterAction(eglGetProcAddress_Call, obj);
}

RetValue GetProcAddressAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    // get entrypoint name
    const char* entrpName;
    call.getArgs()[0].get(entrpName);

    // translate entrypoint to debugger enum
    Entrypoint entryp = GetEntryPointEnum(entrpName);
    if (entryp == NO_ENTRYPOINT) {
        // entrypoint not supported by debugger. retult of native
        // *GetProcAddress
        // will be passed to application, making a hole in trace.

        // TODO: add partial support for unknown entrypoints
        return ret;
    }
    // Load and get address of entrypoint implementation
    if (GlobalState::getApiLoader().loadExtPointer(entryp)) {
        // entrypoint supported by implementation, return address of wrapper to
        // application
        ret = reinterpret_cast<FUNC_PTR>(getWrapperPointer(entryp));
    } else {
        // entrypoint unsupported by implementation, return NULL to application
        ret = (FUNC_PTR)NULL;
    }
    return ret;
}

#if DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)

void SurfaceAction::Register(ActionManager& manager) {
    std::shared_ptr<SurfaceAction> obj
        = std::make_shared<SurfaceAction>();
    manager.RegisterAction(eglCreateWindowSurface_Call, obj);
    manager.RegisterAction(eglCreatePixmapSurface_Call, obj);
    manager.RegisterAction(eglCreatePbufferSurface_Call, obj);
}

void SurfaceAction::Post(const CalledEntryPoint& call, const RetValue& ret) {

    EGLSurface surface;
    ret.get(surface);
    if (surface != EGL_NO_SURFACE) {
        EGLDisplay dpy;
        call.getArgs()[0].get(dpy);
        EGLConfig config;
        call.getArgs()[1].get(config);

        DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                             DGLDisplayState::Type::EGL)
                ->addSurface<dglState::NativeSurfaceEGL>(
                          reinterpret_cast<opaque_id_t>(surface),
                          reinterpret_cast<opaque_id_t>(config));
    }
    PrevPost(call, ret);
}
#endif

void ContextAction::Register(ActionManager& manager) {
    std::shared_ptr<ContextAction> obj
        = std::make_shared<ContextAction>();
    manager.RegisterAction(wglCreateContext_Call, obj);
    manager.RegisterAction(wglCreateLayerContext_Call, obj);
    manager.RegisterAction(wglCreateContextAttribsARB_Call, obj);
    manager.RegisterAction(wglMakeCurrent_Call, obj);
    manager.RegisterAction(wglMakeContextCurrentARB_Call, obj);
    manager.RegisterAction(wglDeleteContext_Call, obj);

    manager.RegisterAction(glXCreateContext_Call, obj);
    manager.RegisterAction(glXCreateNewContext_Call, obj);
    manager.RegisterAction(glXCreateContextAttribsARB_Call, obj);
    manager.RegisterAction(glXMakeCurrent_Call, obj);
    manager.RegisterAction(glXMakeContextCurrent_Call, obj);
    manager.RegisterAction(glXDestroyContext_Call, obj);

    manager.RegisterAction(eglCreateContext_Call, obj);
    manager.RegisterAction(eglMakeCurrent_Call, obj);
    manager.RegisterAction(eglDestroyContext_Call, obj);
    manager.RegisterAction(eglReleaseThread_Call, obj);
    manager.RegisterAction(eglBindAPI_Call, obj);
}

void ContextAction::Post(const CalledEntryPoint& call, const RetValue& ret) {
#ifdef HAVE_LIBRARY_WGL
    HGLRC ctx;
    BOOL retBool;
#endif
#ifdef HAVE_LIBRARY_GLX
    GLXContext ctx;
    GLXDrawable readDrawable, drawDrawable;
    Display* dpy;
    Bool retBool;
#endif
    EGLBoolean eglBool;
    EGLContext eglCtx;

    Entrypoint entryp = call.getEntrypoint();

    switch (entryp) {
#ifdef HAVE_LIBRARY_WGL
        case wglCreateContext_Call:
        case wglCreateLayerContext_Call:
            ret.get(ctx);
            if (NULL != ctx) {
                HDC device;
                call.getArgs()[0].get(device);
                DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                        ->createContext(
                                  dglState::GLContextVersion::Type::DT,
                                  dglState::GLContextCreationData(
                                          entryp,
                                          (opaque_id_t)GetPixelFormat(device),
                                          std::vector<gl_t>()),
                                  reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case wglCreateContextAttribsARB_Call:
            ret.get(ctx);
            if (NULL != ctx) {
                HDC device;
                const int* attribList;
                call.getArgs()[0].get(device);
                call.getArgs()[2].get(attribList);

                dglState::GLContextVersion::Type contextType = 
                    dglState::GLContextVersion::Type::DT;


                std::vector<gl_t> attributes;
                if (attribList) {
                    int i = 0;
                    while (attribList[i]) {
                        attributes.push_back(attribList[i++]);
                        attributes.push_back(attribList[i++]);

                        if (attributes[i - 2] == WGL_CONTEXT_PROFILE_MASK_ARB && 
                            attributes[i - 1] == WGL_CONTEXT_ES_PROFILE_BIT_EXT) {
                                contextType = dglState::GLContextVersion::Type::ES;
                        }
                    }
                }

                DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                        ->createContext(
                                  contextType,
                                  dglState::GLContextCreationData(
                                          entryp,
                                          (opaque_id_t)GetPixelFormat(device),
                                          attributes),
                                  reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case wglMakeCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                HDC device;
                call.getArgs()[0].get(device);
                call.getArgs()[1].get(ctx);

                dglState::NativeSurfaceBase* surface = NULL;
                if (device) {
                    surface =
                            DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                                    ->ensureSurface<dglState::NativeSurfaceWGL>(
                                              (opaque_id_t)device)
                                    ->second.get();
                }

                DGLThreadState::get()->bindContext(
                        DGLDisplayState::defDpy(DGLDisplayState::Type::WGL),
                        reinterpret_cast<opaque_id_t>(ctx), surface, surface);
            }
            break;
        case wglMakeContextCurrentARB_Call:
            ret.get(retBool);
            if (retBool) {
                HDC deviceDraw;
                HDC deviceRead;
                call.getArgs()[0].get(deviceDraw);
                call.getArgs()[1].get(deviceRead);
                call.getArgs()[2].get(ctx);

                dglState::NativeSurfaceBase* surfaceDraw = NULL;
                if (deviceDraw) {
                    surfaceDraw =
                        DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                        ->ensureSurface<dglState::NativeSurfaceWGL>(
                        (opaque_id_t)deviceDraw)
                        ->second.get();
                }
                dglState::NativeSurfaceBase* surfaceRead = NULL;
                if (deviceRead) {
                    surfaceRead =
                        DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                        ->ensureSurface<dglState::NativeSurfaceWGL>(
                        (opaque_id_t)deviceRead)
                        ->second.get();
                }

                DGLThreadState::get()->bindContext(
                    DGLDisplayState::defDpy(DGLDisplayState::Type::WGL),
                    reinterpret_cast<opaque_id_t>(ctx), surfaceDraw, surfaceRead);
            }
            break;
        case wglDeleteContext_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(ctx);
                DGLDisplayState::defDpy(DGLDisplayState::Type::WGL)
                        ->deleteContext(reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
#endif
#ifdef HAVE_LIBRARY_GLX
        case glXCreateContextAttribsARB_Call:
            ret.get(ctx);
            if (ctx) {
                GLXFBConfig config;
                const int* attribList;
                call.getArgs()[0].get(dpy);
                call.getArgs()[1].get(config);
                call.getArgs()[4].get(attribList);

                std::vector<gl_t> attributes;
                dglState::GLContextVersion::Type contextType =
                    dglState::GLContextVersion::Type::DT;

                if (attribList) {
                    int i = 0;
                    while (attribList[i] != None) {
                        attributes.push_back(attribList[i++]);
                        attributes.push_back(attribList[i++]);
                        if (attributes[i - 2] == GLX_CONTEXT_PROFILE_MASK_ARB && 
                            attributes[i - 1] == GLX_CONTEXT_ES_PROFILE_BIT_EXT) {
                                contextType = dglState::GLContextVersion::Type::ES;
                        }
                    }
                }

                DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                                     DGLDisplayState::Type::GLX)
                        ->createContext(contextType,
                                        dglState::GLContextCreationData(
                                                entryp, (opaque_id_t)config,
                                                attributes),
                                        reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case glXCreateContext_Call:
            ret.get(ctx);
            if (ctx) {
                GLXFBConfig* memToFree;
                XVisualInfo* vis;
                call.getArgs()[0].get(dpy);
                call.getArgs()[1].get(vis);
                DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                                     DGLDisplayState::Type::GLX)
                        ->createContext(
                                  dglState::GLContextVersion::Type::DT,
                                  dglState::GLContextCreationData(
                                          entryp,
                                          (opaque_id_t) *
                                                  dglState::NativeSurfaceGLX::
                                                          getFbConfigForVisual(
                                                                  dpy,
                                                                  vis->visualid,
                                                                  &memToFree),
                                          std::vector<gl_t>()),
                                  reinterpret_cast<opaque_id_t>(ctx));
                if (memToFree) {
                    XFree(memToFree);
                }
            }
            break;

        case glXCreateNewContext_Call:
            ret.get(ctx);
            if (ctx) {
                GLXFBConfig config;
                call.getArgs()[0].get(dpy);
                call.getArgs()[1].get(config);
                DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                                     DGLDisplayState::Type::GLX)
                        ->createContext(dglState::GLContextVersion::Type::DT,
                                        dglState::GLContextCreationData(
                                                entryp, (opaque_id_t)config,
                                                std::vector<gl_t>()),
                                        reinterpret_cast<opaque_id_t>(ctx));
            }
            break;
        case glXMakeCurrent_Call:
        case glXMakeContextCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(dpy);

                if (entryp == glXMakeCurrent_Call) {

                    call.getArgs()[1].get(readDrawable);
                    drawDrawable = readDrawable;
                    call.getArgs()[2].get(ctx);

                } else if (entryp == glXMakeContextCurrent_Call) {

                    call.getArgs()[1].get(drawDrawable);
                    call.getArgs()[2].get(readDrawable);
                    call.getArgs()[3].get(ctx);
                }

                dglState::NativeSurfaceBase* readSurface = NULL;
                if (readDrawable) {
                    readSurface =
                            DGLDisplayState::get(
                                    reinterpret_cast<opaque_id_t>(dpy),
                                    DGLDisplayState::Type::GLX)
                                    ->ensureSurface<dglState::NativeSurfaceGLX>(
                                              (opaque_id_t)readDrawable)
                                    ->second.get();
                }
                dglState::NativeSurfaceBase* drawSurface = NULL;
                if (drawDrawable) {
                    drawSurface =
                            DGLDisplayState::get(
                                    reinterpret_cast<opaque_id_t>(dpy),
                                    DGLDisplayState::Type::GLX)
                                    ->ensureSurface<dglState::NativeSurfaceGLX>(
                                              (opaque_id_t)drawDrawable)
                                    ->second.get();
                }
                DGLThreadState::get()->bindContext(
                        DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                                             DGLDisplayState::Type::GLX),
                        reinterpret_cast<opaque_id_t>(ctx), drawSurface,
                        readSurface);
            }
            break;
        case glXDestroyContext_Call:
            call.getArgs()[0].get(dpy);
            call.getArgs()[0].get(ctx);
            DGLDisplayState::get(reinterpret_cast<opaque_id_t>(dpy),
                                 DGLDisplayState::Type::GLX)
                    ->lazyDeleteContext(reinterpret_cast<opaque_id_t>(ctx));
            break;
#endif
        case eglBindAPI_Call:
            ret.get(eglBool);
            if (eglBool) {
                EGLenum apiEnum;
                call.getArgs()[0].get(apiEnum);

                DGLThreadState::BoundEGLApi api = 
                    DGLThreadState::BoundEGLApi::UNKNOWN_API;

                switch (apiEnum) {
                    case EGL_OPENGL_ES_API:
                        api = DGLThreadState::BoundEGLApi::OPENGL_ES_API;
                        break;
                    case EGL_OPENGL_API:
                        api = DGLThreadState::BoundEGLApi::OPENGL_API;
                }
                DGLThreadState::get()->bindEGLApi(api);
                
            }
            break;
        case eglCreateContext_Call:
            ret.get(eglCtx);
            if (NULL != eglCtx) {
                EGLDisplay eglDpy;
                EGLConfig eglConfig;
                EGLint const* attribList;
                call.getArgs()[0].get(eglDpy);
                call.getArgs()[1].get(eglConfig);
                call.getArgs()[3].get(attribList);

                std::vector<gl_t> attributes;
                int major = 1, minor = 1;

                if (attribList) {
                    int i = 0;
                    while (attribList[i] != EGL_NONE) {
                        if (attribList[i] == EGL_CONTEXT_CLIENT_VERSION ||
                            attribList[i] == EGL_CONTEXT_MAJOR_VERSION_KHR) {
                                major = static_cast<int>(attribList[i + 1]);
                        }
                        if (attribList[i] == EGL_CONTEXT_MINOR_VERSION_KHR) {
                            minor = static_cast<int>(attribList[i + 1]);
                        }
                        attributes.push_back(attribList[i++]);
                        attributes.push_back(attribList[i++]);
                    }
                }

                DGLDisplayState* displayState = DGLDisplayState::get((opaque_id_t)eglDpy,
                    DGLDisplayState::Type::EGL);

                dglState::GLContextVersion::Type contextType = dglState::GLContextVersion::Type::UNSUPPORTED;
                if (DGLThreadState::get()->getEGLApi() == DGLThreadState::BoundEGLApi::OPENGL_ES_API) {
                    contextType = dglState::GLContextVersion::Type::ES;
                } else if (DGLThreadState::get()->getEGLApi() == DGLThreadState::BoundEGLApi::OPENGL_API) {
                    contextType = dglState::GLContextVersion::Type::DT;
                } else {
                    contextType = dglState::GLContextVersion::Type::UNSUPPORTED;
                }
                dglState::GLContextVersion version(contextType, major, minor);

                GlobalState::getApiLoader().loadLibraries(version.getNeededApiLibraries(displayState));

                displayState->createContext(
                    dglState::GLContextVersion::Type::ES,
                    dglState::GLContextCreationData(
                    entryp, (opaque_id_t)eglConfig,
                    attributes),
                    reinterpret_cast<opaque_id_t>(eglCtx));
            }
            break;
        case eglMakeCurrent_Call:
            ret.get(eglBool);
            if (eglBool) {
                EGLDisplay eglDpy;
                EGLSurface eglReadSurface;
                EGLSurface eglDrawSurface;
                call.getArgs()[0].get(eglDpy);
                call.getArgs()[1].get(eglDrawSurface);
                call.getArgs()[2].get(eglReadSurface);
                call.getArgs()[3].get(eglCtx);

                dglState::NativeSurfaceBase* readSurface = NULL;
                if (eglReadSurface) {
#if DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)
                    readSurface =
                            DGLDisplayState::get((opaque_id_t)eglDpy,
                                                 DGLDisplayState::Type::EGL)
                                    ->getSurface((opaque_id_t)eglReadSurface)
                                    ->second.get();
#else
                    readSurface =
                            DGLDisplayState::get((opaque_id_t)eglDpy,
                                                 DGLDisplayState::Type::EGL)
                                    ->ensureSurface<dglState::NativeSurfaceEGL>((opaque_id_t)eglReadSurface)
                                    ->second.get();
#endif
                }
                dglState::NativeSurfaceBase* drawSurface = NULL;
                if (eglDrawSurface) {
#if DGL_HAVE_WA(ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID)
                    drawSurface =
                            DGLDisplayState::get((opaque_id_t)eglDpy,
                                                 DGLDisplayState::Type::EGL)
                                    ->getSurface((opaque_id_t)eglDrawSurface)
                                    ->second.get();
#else
                    drawSurface =
                            DGLDisplayState::get((opaque_id_t)eglDpy,
                                                 DGLDisplayState::Type::EGL)
                                    ->ensureSurface<dglState::NativeSurfaceEGL>((opaque_id_t)eglDrawSurface)
                                    ->second.get();
#endif
                }

                DGLThreadState::get()->bindContext(
                        DGLDisplayState::get((opaque_id_t)eglDpy,
                                             DGLDisplayState::Type::EGL),
                        reinterpret_cast<opaque_id_t>(eglCtx), drawSurface,
                        readSurface);
            }
            break;
        case eglDestroyContext_Call:
            ret.get(eglBool);
            if (eglBool) {
                EGLDisplay eglDpy;
                call.getArgs()[0].get(eglDpy);
                call.getArgs()[1].get(eglCtx);
                DGLDisplayState::get((opaque_id_t)eglDpy,
                                     DGLDisplayState::Type::EGL)
                        ->lazyDeleteContext(
                                  reinterpret_cast<opaque_id_t>(eglCtx));
            }
            break;
        case eglReleaseThread_Call:
            ret.get(eglBool);
            if (eglBool) {
                DGLThreadState::releaseAPI();
            }
            break;
    }
    PrevPost(call, ret);
}

void DebugContextAction::Register(ActionManager& manager) {
    std::shared_ptr<DebugContextAction> obj
        = std::make_shared<DebugContextAction>();

    manager.RegisterAction(wglCreateContext_Call, obj);
    manager.RegisterAction(wglCreateLayerContext_Call, obj);
    manager.RegisterAction(wglCreateContextAttribsARB_Call, obj);
   
    manager.RegisterAction(glXCreateContext_Call, obj);
    manager.RegisterAction(glXCreateNewContext_Call, obj);
    manager.RegisterAction(glXCreateContextAttribsARB_Call, obj);
   
    //TODO:
    //manager.RegisterAction(eglCreateContext_Call, obj);
}

bool DebugContextAction::anyContextPresent = false;

RetValue DebugContextAction::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    if (!GlobalState::getConfiguration().m_ForceDebugContext) {
        return ret;
    }

#ifdef HAVE_LIBRARY_WGL
    HDC hdc = NULL;
    HGLRC sharedCtx = NULL;
    const int* attribList = NULL;
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
            DGL_ASSERT(0);
    }

    std::vector<int> newAttribList;
    bool done = false;
    if (attribList != NULL) {
        int i = 0;
        while (attribList[i]) {
            int attrib = attribList[i++], value = attribList[i++];
            if (attrib == WGL_CONTEXT_FLAGS_ARB) {
                if (!GlobalState::getConfiguration().m_ForceDebugContextES &&
                    (value & WGL_CONTEXT_ES2_PROFILE_BIT_EXT)) {
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
        // we must create one dummy ctx, to force ICD loading on Windows
        // otherwise wglCreateContextAttribsARB, which is an extension, will not
        // be availiable
        tmpCtx = DIRECT_CALL_CHK(wglCreateContext)(hdc);
        DIRECT_CALL_CHK(wglMakeCurrent)(hdc, tmpCtx);
    }

    // call wglCreateContextAttribsARB only if supported by implementation.
    // Otherwise do nothing - ctx will be created in wrapper function
    if (POINTER(wglCreateContextAttribsARB) ||
        GlobalState::getApiLoader().loadExtPointer(wglCreateContextAttribsARB_Call)) {
        ret = DIRECT_CALL_CHK(wglCreateContextAttribsARB)(hdc, sharedCtx,
                                                          &newAttribList[0]);
    }

    if (tmpCtx) {
        // unwind dummy ctx
        DIRECT_CALL_CHK(wglMakeCurrent)(NULL, NULL);
        DIRECT_CALL_CHK(wglDeleteContext)(tmpCtx);
        anyContextPresent = true;
    }
#elif defined(HAVE_LIBRARY_GLX)
    Display* dpy = NULL;
    GLXFBConfig config = NULL;
    GLXContext sharedContext = NULL;
    Bool direct = True;
    const int* attribList = NULL;
    int renderType;    // not really used

    XVisualInfo* vis;

    GLXFBConfig* memToFree = NULL;

    switch (call.getEntrypoint()) {
        case glXCreateContext_Call:
            call.getArgs()[0].get(dpy);
            call.getArgs()[1].get(vis);
            call.getArgs()[2].get(sharedContext);
            call.getArgs()[3].get(direct);
            config = *dglState::NativeSurfaceGLX::getFbConfigForVisual(
                              dpy, vis->visualid, &memToFree);
            DGL_ASSERT(config);
            break;
        case glXCreateNewContext_Call:
            call.getArgs()[0].get(dpy);
            call.getArgs()[1].get(config);
            call.getArgs()[2].get(renderType);
            call.getArgs()[3].get(sharedContext);
            call.getArgs()[4].get(direct);
            break;
        case glXCreateContextAttribsARB_Call:
            call.getArgs()[0].get(dpy);
            call.getArgs()[1].get(config);
            call.getArgs()[2].get(sharedContext);
            call.getArgs()[3].get(direct);
            call.getArgs()[4].get(attribList);
            break;
        default:
            DGL_ASSERT(0);
    }

    std::vector<int> newAttribList;
    bool done = false;
    if (attribList != NULL) {
        int i = 0;
        while (attribList[i]) {
            int attrib = attribList[i++], value = attribList[i++];
            if (attrib == GLX_CONTEXT_FLAGS_ARB) {
                if (!GlobalState::getConfiguration().m_ForceDebugContextES &&
                    (value & GLX_CONTEXT_ES2_PROFILE_BIT_EXT)) {
                    return ret;
                }
                value |= GLX_CONTEXT_DEBUG_BIT_ARB;
                done = true;
            }
            newAttribList.push_back(attrib);
            newAttribList.push_back(value);
        }
    }
    newAttribList.push_back(0);
    if (!done) {
        newAttribList[newAttribList.size() - 1] = GLX_CONTEXT_FLAGS_ARB;
        newAttribList.push_back(GLX_CONTEXT_DEBUG_BIT_ARB);
        newAttribList.push_back(0);
    }

    // call glXCreateContextAttribsARB only if supported by implementation.
    // Otherwise do nothing - ctx will be created in wrapper function
    if (POINTER(glXCreateContextAttribsARB) ||
        GlobalState::getApiLoader().loadExtPointer(glXCreateContextAttribsARB_Call)) {
        ret = DIRECT_CALL_CHK(glXCreateContextAttribsARB)(
                dpy, config, sharedContext, direct, &newAttribList[0]);
    }

    if (memToFree) {
        XFree(memToFree);
    }
#endif
    return ret;
}

void TextureAction::Register(ActionManager& manager) {
    std::shared_ptr<TextureAction> obj
        = std::make_shared<TextureAction>();

    manager.RegisterAction(glGenTextures_Call, obj);
    manager.RegisterAction(glGenTexturesEXT_Call, obj);
    manager.RegisterAction(glDeleteTextures_Call, obj);
    manager.RegisterAction(glDeleteTexturesEXT_Call, obj);
    manager.RegisterAction(glBindTexture_Call, obj);
    manager.RegisterAction(glBindTextureEXT_Call, obj);
}

void TextureAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenTextures_Call || entrp == glGenTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().getShared()->get().m_Textures.getOrCreateObject<void>(names[i]);
            }
        } else if (entrp == glDeleteTextures_Call ||
                   entrp == glDeleteTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->shadow().getTexUnits().unbindTexture(names[i]);
                gc->ns().getShared()->get().m_Textures.deleteObject(names[i]);
            }
        } else if (entrp == glBindTexture_Call ||
                   entrp == glBindTextureEXT_Call) {
            GLenum target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            gc->ns().getShared()->get().m_Textures.getOrCreateObject<void>(name)->setTarget(target);
            gc->shadow().getTexUnits().bindTexture(target, name);
        }
    }
    PrevPost(call, ret);
}

void TextureFormatAction::Register(ActionManager& manager) {
    std::shared_ptr<TextureFormatAction> obj
        = std::make_shared<TextureFormatAction>();

    manager.RegisterAction(glTexImage1D_Call, obj);
    manager.RegisterAction(glTexImage2D_Call, obj);
    manager.RegisterAction(glTexImage2DMultisample_Call, obj);
    manager.RegisterAction(glTexImage3D_Call, obj);
    manager.RegisterAction(glTexImage3DEXT_Call, obj);
    manager.RegisterAction(glTexImage3DOES_Call, obj);
    manager.RegisterAction(glTexImage3DMultisample_Call, obj);
    manager.RegisterAction(glTexStorage1D_Call, obj);
    manager.RegisterAction(glTexStorage2D_Call, obj);
    manager.RegisterAction(glTexStorage2DMultisample_Call, obj);
    manager.RegisterAction(glTexStorage3D_Call, obj);
    manager.RegisterAction(glTexStorage3DMultisample_Call, obj);
    manager.RegisterAction(glTexStorage1DEXT_Call, obj);
    manager.RegisterAction(glTexStorage2DEXT_Call, obj);
    manager.RegisterAction(glTexStorage3DEXT_Call, obj);
}

void TextureFormatAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        //The type of this parameter is a complete disaster. Expect some 
        //TexImage/TexStorage functions to provide int, some to provide enum. 
        //At this moment codegen just passes type used in call - Enum info is stored
        //in other place...
        GLenum iFormat = 0;
        GLint tmpiFormat_asInt = 0;

        GLenum format = 0, type = 0;
        GLint level = 0;
        GLsizei width = 1, height = 1, depth = 1;
        GLenum target;
        
        bool immutable = false;

        call.getArgs()[0].get(target); 
        switch (entrp) {
            case glTexImage1D_Call:
                call.getArgs()[1].get(level);
                call.getArgs()[2].get(tmpiFormat_asInt);   //GLint
                iFormat = tmpiFormat_asInt;
                call.getArgs()[3].get(width);
                call.getArgs()[call.getArgs().size() - 3].get(format);
                call.getArgs()[call.getArgs().size() - 2].get(type);
                break;
            case glTexImage2D_Call:
                call.getArgs()[1].get(level);
                call.getArgs()[2].get(tmpiFormat_asInt);    //GLint
                iFormat = tmpiFormat_asInt;
                call.getArgs()[3].get(width);
                call.getArgs()[4].get(height);
                call.getArgs()[call.getArgs().size() - 3].get(format);
                call.getArgs()[call.getArgs().size() - 2].get(type);
                break;
            case glTexImage2DMultisample_Call:
                call.getArgs()[2].get(iFormat); //GLenum (genius!)
                call.getArgs()[3].get(width);
                call.getArgs()[4].get(height);
                break;
            case glTexImage3D_Call:    
                call.getArgs()[1].get(level);
                call.getArgs()[2].get(tmpiFormat_asInt);    //GLint
                iFormat = tmpiFormat_asInt;
                call.getArgs()[3].get(width);
                call.getArgs()[4].get(height);
                call.getArgs()[5].get(depth);
                call.getArgs()[call.getArgs().size() - 3].get(format);
                call.getArgs()[call.getArgs().size() - 2].get(type);
                break;
            case glTexImage3DEXT_Call: //Glenum <iformat> (seriously, what-da-fak!)
            case glTexImage3DOES_Call:
                call.getArgs()[1].get(level);
                call.getArgs()[2].get(iFormat);   //GLenum (and you are fired.)
                call.getArgs()[3].get(width);
                call.getArgs()[4].get(height);
                call.getArgs()[5].get(depth);
                call.getArgs()[call.getArgs().size() - 3].get(format);
                call.getArgs()[call.getArgs().size() - 2].get(type);
            break;
            case glTexImage3DMultisample_Call:
                call.getArgs()[2].get(iFormat);   //GLenum
                call.getArgs()[3].get(width);
                call.getArgs()[4].get(height);
                call.getArgs()[5].get(depth);
                break;

            case glTexStorage3DEXT_Call:  //GLenum <iformat>
            case glTexStorage3D_Call:     //GLenum <iformat>
                call.getArgs()[5].get(depth);
                //fall through
            case glTexStorage2DEXT_Call:  //Glenum <iformat>  (so far, so glenum!)
            case glTexStorage2D_Call:     //GLenum <iformat>
                call.getArgs()[4].get(height);
                //fall through
            case glTexStorage1DEXT_Call:  //GLenum
            case glTexStorage1D_Call:
                call.getArgs()[1].get(level);
                call.getArgs()[2].get(iFormat);   //GLenum
                call.getArgs()[3].get(width);
                immutable = true;
                break;

            case glTexStorage3DMultisample_Call:  //GLenum
                call.getArgs()[5].get(depth);
                //fall through
            case glTexStorage2DMultisample_Call:
                call.getArgs()[4].get(height);
                call.getArgs()[2].get(iFormat);  //GLenum
                call.getArgs()[3].get(width);
                immutable = true;
            break;
        }

        GLuint textureName;
        if (glutils::getBoundTexture(glutils::textTargetToBindableTarget(target), textureName)) {

            dglState::GLTextureObj* tex = gc->ns().getShared()->get().m_Textures.getOrCreateObject<void>(textureName);

            tex->setTarget(target);

            if (immutable) {
                tex->setTexStorage(level, width, height, depth, iFormat, format, type);
            } else {
                tex->setTexImage(level, width, height, depth, iFormat, format, type);
            }

        }
        
    }       
    PrevPost(call, ret);
}

void BufferAction::Register(ActionManager& manager) {
    std::shared_ptr<BufferAction> obj
        = std::make_shared<BufferAction>();

    manager.RegisterAction(glGenBuffers_Call, obj);
    manager.RegisterAction(glGenBuffersARB_Call, obj);
    manager.RegisterAction(glDeleteBuffers_Call, obj);
    manager.RegisterAction(glDeleteBuffersARB_Call, obj);
    manager.RegisterAction(glBindBuffer_Call, obj);
    manager.RegisterAction(glBindBufferARB_Call, obj);
}

void BufferAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenBuffers_Call || entrp == glGenBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().getShared()->get().m_Buffers.getOrCreateObject<void>(names[i]);
            }
        } else if (entrp == glDeleteBuffers_Call ||
                   entrp == glDeleteBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().getShared()->get().m_Buffers.deleteObject(names[i]);
            }
        } else if (entrp == glBindBuffer_Call ||
                   entrp == glBindBufferARB_Call) {
            GLenum target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                gc->ns().getShared()->get().m_Buffers.getOrCreateObject<void>(name)->setTarget(target);
            }
        }
    }
    PrevPost(call, ret);
}

void ProgramAction::Register(ActionManager& manager) {
    std::shared_ptr<ProgramAction> obj
        = std::make_shared<ProgramAction>();

    manager.RegisterAction(glCreateProgram_Call, obj);
    manager.RegisterAction(glCreateProgramObjectARB_Call, obj);
    manager.RegisterAction(glDeleteProgram_Call, obj);
    manager.RegisterAction(glDeleteObjectARB_Call, obj);
    manager.RegisterAction(glUseProgram_Call, obj);
    manager.RegisterAction(glUseProgramObjectARB_Call, obj);
    manager.RegisterAction(glLinkProgram_Call, obj);
    manager.RegisterAction(glLinkProgramARB_Call, obj);

    manager.RegisterAction(glCreateShaderProgramv_Call, obj); //TODO: add suffixed version
}

void ProgramAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (gc) {
        GLuint name;
        if (entrp == glCreateProgram_Call ||
            entrp == glCreateProgramObjectARB_Call) {

            ret.get(name);

            gc->ns().m_Programs.getOrCreateObject(name, entrp == glCreateProgramObjectARB_Call);

        } else if (entrp == glDeleteProgram_Call ||
                   entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLProgramObj* program =
                    gc->ns().m_Programs.getOrCreateObject(name, entrp == glDeleteObjectARB_Call);
            program->markDeleted();
            if (program->mayDelete()) {
                gc->ns().m_Programs.deleteObject(name);
            }

        } else if (entrp == glUseProgram_Call ||
                   entrp == glUseProgramObjectARB_Call) {

            call.getArgs()[0].get(name);

            GLuint lastProgram = gc->shadow().m_CurrentProgram;

            if (lastProgram != 0 && lastProgram != name) {

                // we may delete last program, if marked for deletion

                dglState::GLProgramObj* currentProgram =
                        gc->ns().m_Programs.getOrCreateObject(lastProgram,
                                          entrp == glUseProgramObjectARB_Call);

                currentProgram->setInUse(false);
                if (currentProgram->mayDelete()) {
                    gc->ns().m_Programs.deleteObject(lastProgram);
                }
            }

            gc->shadow().m_CurrentProgram = name;

            if (name != 0) {
                gc->ns().m_Programs.getOrCreateObject(name, entrp == glUseProgramObjectARB_Call)
                        ->setInUse(true);
            }
        } else if (entrp == glLinkProgram_Call) {

            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetProgramiv)(name, GL_LINK_STATUS, &linkStatus);

            if (linkStatus != GL_TRUE) {
                GlobalState::getDebugController().getBreakState().setBreakAtCompilerError();
            }

        } else if (entrp == glLinkProgramARB_Call) {
            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetObjectParameterivARB)(
                    name, GL_OBJECT_LINK_STATUS_ARB, &linkStatus);

            if (linkStatus != GL_TRUE) {
                GlobalState::getDebugController().getBreakState().setBreakAtCompilerError();
            }
        } else if (entrp == glCreateShaderProgramv_Call) {

                ret.get(name);

                GLsizei count; 
                const void* strings;

                call.getArgs()[1].get(count);
                call.getArgs()[2].get(strings);

                dglState::GLProgramObj* program = 
                    gc->ns().m_Programs.getOrCreateObject(name, false /* TODO: ??? */);

                program->setEmbeddedSSOSource(count, static_cast<const char* const*>(strings));
        }
    }
    PrevPost(call, ret);
}

void ProgramPipelineAction::Register(ActionManager& /*manager*/) {
    std::shared_ptr<ProgramPipelineAction> obj
        = std::make_shared<ProgramPipelineAction>();

    //TODO: add suffixes & enable

    //manager.RegisterAction(glGenProgramPipelines_Call, obj);
    //
    //manager.RegisterAction(glBindProgramPipeline_Call, obj);
    //manager.RegisterAction(glUseProgramStages_Call, obj);
    //manager.RegisterAction(glDeleteProgramPipelines_Call, obj);
}

void ProgramPipelineAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (gc) {
       
        if (entrp == glGenProgramPipelines_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().m_ProgramPipelines.getOrCreateObject<void>(names[i]);
            }
        } else if (entrp == glDeleteProgramPipelines_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().m_ProgramPipelines.deleteObject(names[i]);
            }
        } else if (entrp == glBindProgramPipeline_Call) {
            GLuint name;
            call.getArgs()[0].get(name);
            gc->ns().m_ProgramPipelines.getOrCreateObject<void>(name);
        } else if (entrp == glUseProgramStages_Call) {
            GLuint pipelineName;
            GLbitfield stages;
            GLuint program;

            call.getArgs()[0].get(pipelineName);
            call.getArgs()[1].get(stages);
            call.getArgs()[2].get(program);

            gc->ns().m_ProgramPipelines.getOrCreateObject<void>(pipelineName)->useProgramStages(stages, program);

        }
    }
    PrevPost(call, ret);
}

void ShaderAction::Register(ActionManager& manager) {
    std::shared_ptr<ShaderAction> obj
        = std::make_shared<ShaderAction>();

    manager.RegisterAction(glCreateShader_Call, obj);
    manager.RegisterAction(glCreateShaderObjectARB_Call, obj);
    manager.RegisterAction(glDeleteShader_Call, obj);
    manager.RegisterAction(glDeleteObjectARB_Call, obj);
    manager.RegisterAction(glCompileShader_Call, obj);
    manager.RegisterAction(glCompileShaderARB_Call, obj);
    manager.RegisterAction(glAttachObjectARB_Call, obj);
    manager.RegisterAction(glAttachShader_Call, obj);
    manager.RegisterAction(glDetachObjectARB_Call, obj);
    manager.RegisterAction(glDetachShader_Call, obj);
    manager.RegisterAction(glShaderSource_Call, obj);
    manager.RegisterAction(glShaderSourceARB_Call, obj);
}

void ShaderAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (gc) {
        GLuint name;
        if (entrp == glCreateShader_Call ||
            entrp == glCreateShaderObjectARB_Call) {

            // we assume that GLhandleARB is the same type as GLuint

            ret.get(name);

            GLenum target;
            call.getArgs()[0].get(target);

            gc->ns().m_Shaders.getOrCreateObject(name, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glCreateShaderObjectARB_Call))
                    ->createCalled(target);

        } else if (entrp == glDeleteShader_Call ||
                   entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLShaderObj* shader =
                    gc->ns().m_Shaders.getOrCreateObject(name, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glDeleteObjectARB_Call));

            shader->deleteCalled();

        } else if (entrp == glCompileShader_Call ||
                   entrp == glCompileShaderARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLShaderObj* shader =
                    gc->ns().m_Shaders.getOrCreateObject(name, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glCompileShaderARB_Call));
            GLint compileStatus = shader->queryCompilationStatus();

            if (compileStatus != GL_TRUE) {
                GlobalState::getDebugController().getBreakState().setBreakAtCompilerError();
            }

        } else if (entrp == glAttachShader_Call ||
                   entrp == glAttachObjectARB_Call) {

            GLuint prog, shad;
            call.getArgs()[0].get(prog);
            call.getArgs()[1].get(shad);
            gc->ns().m_Programs.getOrCreateObject(prog, entrp == glAttachObjectARB_Call)
                    ->attachShader(gc->ns().m_Shaders.getOrCreateObject(
                              shad, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glAttachObjectARB_Call)));

        } else if (entrp == glDetachShader_Call ||
                   entrp == glDetachObjectARB_Call) {

            GLuint prog, shad;
            call.getArgs()[0].get(prog);
            call.getArgs()[1].get(shad);
            gc->ns().m_Programs.getOrCreateObject(prog, entrp == glDetachObjectARB_Call)
                    ->detachShader(gc->ns().m_Shaders.getOrCreateObject(
                              shad, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glAttachObjectARB_Call)));

        } else if (entrp == glShaderSourceARB_Call ||
                   entrp == glShaderSource_Call) {

            GLuint shad;
            call.getArgs()[0].get(shad);
            gc->ns().m_Shaders.getOrCreateObject(shad, dglState::GLShaderObj::GLShaderObjCreateData(&gc->ns(), entrp == glShaderSourceARB_Call))
                    ->shaderSourceCalled();
        }
    }
    PrevPost(call, ret);
}

void ImmediateModeAction::Register(ActionManager& manager) {
    std::shared_ptr<ImmediateModeAction> obj
        = std::make_shared<ImmediateModeAction>();

    manager.RegisterAction(glBegin_Call, obj);
    manager.RegisterAction(glEnd_Call, obj);
}

void ImmediateModeAction::NoGLErrorPost(const CalledEntryPoint& call,
                               const RetValue& ret) {
    if (gc) {
        switch (call.getEntrypoint()) {
            case glBegin_Call:
                gc->shadow().setImmediateMode(true);
                break;
            case glEnd_Call:
                gc->shadow().setImmediateMode(false);
                break;
        }
    }
    PrevPost(call, ret);
}

void FBOAction::Register(ActionManager& manager) {
    std::shared_ptr<FBOAction> obj
        = std::make_shared<FBOAction>();

    manager.RegisterAction(glGenFramebuffers_Call, obj);
    manager.RegisterAction(glGenFramebuffersEXT_Call, obj);
    manager.RegisterAction(glDeleteFramebuffers_Call, obj);
    manager.RegisterAction(glDeleteFramebuffersEXT_Call, obj);
    manager.RegisterAction(glBindFramebuffer_Call, obj);
    manager.RegisterAction(glBindFramebufferEXT_Call, obj);
}

void FBOAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenFramebuffers_Call ||
            entrp == glGenFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().m_FBOs.getOrCreateObject<void>(names[i]);
            }
        } else if (entrp == glDeleteFramebuffers_Call ||
                   entrp == glDeleteFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                gc->ns().m_FBOs.deleteObject(names[i]);
            }
        } else if (entrp == glBindFramebuffer_Call ||
                   entrp == glBindFramebufferEXT_Call) {
            GLenum target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                gc->ns().m_FBOs.getOrCreateObject<void>(name);
            }
        }
    }
    PrevPost(call, ret);
}

void RenderbufferAction::Register(ActionManager& manager) {
    std::shared_ptr<RenderbufferAction> obj
        = std::make_shared<RenderbufferAction>();

    manager.RegisterAction(glGenRenderbuffers_Call, obj);
    manager.RegisterAction(glGenRenderbuffersEXT_Call, obj);
    manager.RegisterAction(glDeleteRenderbuffers_Call, obj);
    manager.RegisterAction(glDeleteRenderbuffersEXT_Call, obj);
    manager.RegisterAction(glBindRenderbuffer_Call, obj);
    manager.RegisterAction(glBindRenderbufferEXT_Call, obj);
}

void RenderbufferAction::NoGLErrorPost(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (gc) {

        if (entrp == glGenRenderbuffers_Call ||
            entrp == glGenRenderbuffersEXT_Call) {
                GLsizei n = 0;
                call.getArgs()[0].get(n);

                GLuint* names;
                call.getArgs()[1].get(names);

                for (GLsizei i = 0; i < n; i++) {
                    gc->ns().m_Renderbuffers.getOrCreateObject<void>(names[i]);
                }
        } else if (entrp == glDeleteRenderbuffers_Call ||
                   entrp == glDeleteRenderbuffersEXT_Call) {
                GLsizei n = 0;
                call.getArgs()[0].get(n);

                const GLuint* names;
                call.getArgs()[1].get(names);

                for (GLsizei i = 0; i < n; i++) {
                    gc->ns().m_Renderbuffers.deleteObject(names[i]);
                }
        } else if (entrp == glBindRenderbuffer_Call ||
                   entrp == glBindRenderbufferEXT_Call) {
                GLenum target;
                call.getArgs()[0].get(target);
                GLuint name;
                call.getArgs()[1].get(name);
                if (name) {
                    gc->ns().m_Renderbuffers.getOrCreateObject<void>(name);
                }
        }
    }
    PrevPost(call, ret);
}

void DebugOutputCallback::Register(ActionManager& manager) {
    std::shared_ptr<DebugOutputCallback> obj
        = std::make_shared<DebugOutputCallback>();

    manager.RegisterAction(glDebugMessageCallback_Call, obj);
    manager.RegisterAction(glDebugMessageCallbackARB_Call, obj);
}

RetValue DebugOutputCallback::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    Entrypoint entrp = call.getEntrypoint();
    if (gc && (entrp == glDebugMessageCallback_Call ||
               entrp == glDebugMessageCallbackARB_Call)) {
        FUNC_PTR callback;
        const GLvoid* userParam;
        call.getArgs()[0].get(callback);
        call.getArgs()[1].get(userParam);

        // register application's callback into out context
        gc->setCustomDebugOutputCallback((GLDEBUGPROC)callback);

        // call debug message callback now, to prevent override
        if (entrp == glDebugMessageCallback_Call) {
            DIRECT_CALL_CHK(glDebugMessageCallback)(
                    dglState::GLContext::debugOutputCallback, userParam);
        } else if (entrp == glDebugMessageCallbackARB_Call) {
            DIRECT_CALL_CHK(glDebugMessageCallbackARB)(
                    dglState::GLContext::debugOutputCallback, userParam);
        } else {
            DGL_ASSERT(0);
        }

        return RetValue::getVoidAlreadySet();
    }
    return ret;
}

} //namespace actions
