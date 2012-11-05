#include <DGLNet/server.h>

#include <boost/make_shared.hpp>

#include "gl-wrappers.h"
#include "debugger.h"
#include "tracer.h"
#include "pointers.h"
#include "api-loader.h"


boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];


RetValue DefaultTracer::Pre(const CalledEntryPoint& call) {
    g_Controller->getServer().lock();

    //do a fast non-blocking poll to get "interrupt" message, etc.."
    g_Controller->getServer().poll();

    if (g_Controller->getBreakState().isBreaked()) {
        //we just hit a break;
        dglstate::GLContext* ctx = g_GLState.getCurrent();
        dglnet::BreakedCallMessage callStateMessage(call, g_Controller->getCallHistory().size(), ctx?ctx->getId():0, g_GLState.describe());
        g_Controller->getServer().sendMessage(&callStateMessage);
    }
    
    while (g_Controller->getBreakState().isBreaked()) {
        //block & loop until someone unbreaks us
        g_Controller->getServer().run_one();
    }

    g_Controller->getCallHistory().add(call);

    g_Controller->getBreakState().endStep();
    g_Controller->getServer().unlock();
    return RetValue();
}

void DefaultTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {}

RetValue GetProcAddressTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);

    if (ret.isSet()) return ret;

    Entrypoint entryp;
    const char* entrpName; call.getArgs()[0].get(entrpName);
    try {
        entryp = GetEntryPointEnum(entrpName);
    } catch(const std::runtime_error&) {
        //we do not support this entrypoint
        //TODO: add partial support for unknown entrypoints
        return ret;  
    }
    //we recognize this entrypoint, load if nessesary and return address to  wrapper
    LoadOpenGLExtPointer(entryp);
    ret = getWrapperPointer(entryp);
    return ret;
}

void GetProcAddressTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    DefaultTracer::Post(call, ret);
}

RetValue ContextTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);
    return ret;
}

void ContextTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    HGLRC ctx;; 
    BOOL retBool;
   
    switch (call.getEntrypoint()) {
        case wglCreateContext_Call:
            ret.get(ctx);
            if (NULL != ctx) {
                g_GLState.ensureContext(reinterpret_cast<int32_t>(ctx));
            }
            break;
        case wglMakeCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[1].get(ctx);
                g_GLState.bindContext(reinterpret_cast<int32_t>(ctx));
            }
            break;
        case wglDeleteContext_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(ctx);
                g_GLState.deleteContext(reinterpret_cast<int32_t>(ctx));
            }
            break;
    }
    DefaultTracer::Post(call, ret);
}

RetValue TextureTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);
    return ret;
}

void TextureTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (g_GLState.getCurrent()) {

        if (entrp == glGenTextures_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_GLState.getCurrent()->ensureTexture(names[i]);
            }
        } else if (entrp == glDeleteTextures_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_GLState.getCurrent()->deleteTexture(names[i]);
            }
        } else if (entrp == glBindTexture_Call) {
            GLuint name;
            call.getArgs()[1].get(name);
            g_GLState.getCurrent()->ensureTexture(name);
        }
    }
    DefaultTracer::Post(call, ret);
}

RetValue BufferTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);
    return ret;
}

void BufferTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (g_GLState.getCurrent()) {

        if (entrp == glGenBuffers_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_GLState.getCurrent()->ensureBuffer(names[i]);
            }
        } else if (entrp == glDeleteBuffers_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_GLState.getCurrent()->deleteBuffer(names[i]);
            }
        } else if (entrp == glBindBuffer_Call) {
            GLuint name;
            call.getArgs()[1].get(name);
            g_GLState.getCurrent()->ensureBuffer(name);
        }
    }
    DefaultTracer::Post(call, ret);
}

RetValue ProgramTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);
    
    if (call.getEntrypoint() == glUseProgram_Call) {

        GLint currentProgramName;
        DIRECT_CALL(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProgramName);

        dglstate::GLProgramObj* currentProgram = g_GLState.getCurrent()->ensureProgram(currentProgramName);

        currentProgram->use(false);
        if (currentProgram->mayDelete()) {
            g_GLState.getCurrent()->deleteProgram(currentProgramName);
        }
    }
    return ret;
}

void ProgramTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (g_GLState.getCurrent()) {
        GLuint name;
        if (entrp == glCreateProgram_Call) {

            ret.get(name);

            g_GLState.getCurrent()->ensureProgram(name);

        } else if (entrp == glDeleteProgram_Call) {

            call.getArgs()[0].get(name);

            dglstate::GLProgramObj* program = g_GLState.getCurrent()->ensureProgram(name);
            program->markDeleted();
            if (program->mayDelete()) {
                g_GLState.getCurrent()->deleteProgram(name);
            }

        } else if (entrp == glUseProgram_Call) {

            call.getArgs()[0].get(name);

            g_GLState.getCurrent()->ensureProgram(name)->use(true);

        }
    }
    DefaultTracer::Post(call, ret);
}
