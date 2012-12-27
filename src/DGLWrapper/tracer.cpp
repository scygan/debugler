#include <DGLNet/server.h>

#include <boost/make_shared.hpp>

#include "gl-wrappers.h"
#include "debugger.h"
#include "tracer.h"
#include "pointers.h"
#include "api-loader.h"

boost::shared_ptr<TracerBase> g_Tracers[NUM_ENTRYPOINTS];

boost::thread_specific_ptr<int> TracerBase::m_ThreadedInfiniteRecursionGuard;


RetValue TracerBase::DoPre(const CalledEntryPoint& call) {
    if (m_ThreadedInfiniteRecursionGuard.get() == NULL) {
        m_ThreadedInfiniteRecursionGuard.reset(new int(0));
    }

    (*m_ThreadedInfiniteRecursionGuard.get())++;

    if (*m_ThreadedInfiniteRecursionGuard.get() > 1) {
        //	This is unlikely, but may happen sometimes - OpenGL implementation called us. 
        //If we dont catch it here, we will deadlock later, or likely get into infinite recursion.
        return RetValue();
    } else {
        return Pre(call);
    }
}

void TracerBase::DoPost(const CalledEntryPoint& call, const RetValue& ret) {

    (*m_ThreadedInfiniteRecursionGuard.get())--;

    if (*m_ThreadedInfiniteRecursionGuard.get() == 0) {
        Post(call, ret);
    }
}


void TracerBase::SetPrev(const boost::shared_ptr<TracerBase>& prev) {
    m_PrevTracer = prev;
}

RetValue TracerBase::Pre(const CalledEntryPoint& call) {
    return PrevPre(call);
}

void TracerBase::Post(const CalledEntryPoint& call, const RetValue& ret) {
    return PrevPost(call, ret);
}


RetValue TracerBase::PrevPre(const CalledEntryPoint& call) {
    if (m_PrevTracer)
        return m_PrevTracer->Pre(call);
    return RetValue();
}

void TracerBase::PrevPost(const CalledEntryPoint& call, const RetValue& ret) {
    if (m_PrevTracer)
        m_PrevTracer->Post(call, ret);
}

RetValue DefaultTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    g_Controller->getServer().lock();

    //do a fast non-blocking poll to get "interrupt" message, etc.."
    g_Controller->getServer().poll();

    //check if any break is pending
    if (g_Controller->getBreakState().breakAt(call.getEntrypoint())) {
        //we just hit a break;
        dglState::GLContext* ctx = g_DGLGLState.getCurrent();
        dglnet::BreakedCallMessage callStateMessage(call, g_Controller->getCallHistory().size(), ctx?ctx->getId():0, g_DGLGLState.describe());
        g_Controller->getServer().sendMessage(&callStateMessage);
    }
    
    while (g_Controller->getBreakState().isBreaked()) {
        //iterate block & loop until someone unbreaks us
        g_Controller->getServer().run_one();
    }

    //now there should be no breaks

    //add call to history ring
    g_Controller->getCallHistory().add(call);

    g_Controller->getServer().unlock();
    return ret;
}

void DefaultTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    g_Controller->getServer().lock();

    GLenum error;
    if (dglState::GLContext* ctx = g_DGLGLState.getCurrent()) {

        bool hasDebugOutput = ctx->hasDebugOutput();
        if (hasDebugOutput) {
            g_Controller->getCallHistory().setDebugOutput(ctx->popDebugOutput());
        }

        if ((error = g_DGLGLState.getCurrent()->peekError()) != GL_NO_ERROR && g_Config.m_BreakOnGLError) {
            g_Controller->getCallHistory().setError(error);
            g_Controller->getBreakState().breakAt(call.getEntrypoint(), error);
        }
        if (hasDebugOutput && g_Config.m_BreakOnDebugOutput) {
            g_Controller->getBreakState().breakAtDebugOutput();
        }
    }    
    
    g_Controller->getServer().unlock();
    
    PrevPost(call, ret);
}



RetValue GLGetErrorTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);
    
    if (ret.isSet()) return ret;

    if (g_DGLGLState.getCurrent() && call.getEntrypoint() == glGetError_Call) {
        std::pair<bool, GLenum> pokedError = g_DGLGLState.getCurrent()->getPokedError();
        if (pokedError.first) {
            ret = pokedError.second;
        }
    }

    return ret;
}

RetValue GetProcAddressTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    Entrypoint entryp;
    const char* entrpName; call.getArgs()[0].get(entrpName);
    entryp = GetEntryPointEnum(entrpName);
    if (entryp == NO_ENTRYPOINT) {
        //we do not support this entrypoint
        //TODO: add partial support for unknown entrypoints
        return ret; 
    }
    //we recognize this entrypoint, load if nessesary and return address to  wrapper
    LoadOpenGLExtPointer(entryp);
    ret = (const void *)getWrapperPointer(entryp);
    return ret;
}

void ContextTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    HGLRC ctx;
    HDC device;
    BOOL retBool;
   
    switch (call.getEntrypoint()) {
        case wglCreateContext_Call:
            ret.get(ctx);
            if (NULL != ctx) {
                g_DGLGLState.ensureContext(reinterpret_cast<int32_t>(ctx));
            }
            break;
        case wglMakeCurrent_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(device);
                call.getArgs()[1].get(ctx);
                g_DGLGLState.bindContext(reinterpret_cast<uint32_t>(ctx), reinterpret_cast<uint32_t>(device));
            }
            break;
        case wglDeleteContext_Call:
            ret.get(retBool);
            if (retBool) {
                call.getArgs()[0].get(ctx);
                g_DGLGLState.deleteContext(reinterpret_cast<int32_t>(ctx));
            }
            break;
    }
    PrevPost(call, ret);
}

bool DebugContextTracer::anyContextPresent = false;

RetValue DebugContextTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (ret.isSet()) return ret;

    HDC hdc;
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
    }

    std::vector<int> newAttribList; 
    bool done = false;
    if (attribList != NULL) {
        int i = 0; 
        while (attribList[i]) {
            int attrib = attribList[i++], value = attribList[i++]; 
            if (attrib == WGL_CONTEXT_FLAGS_ARB) {
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

    HGLRC tmpCtx;

    if (!anyContextPresent) {
        //we must create one dummy ctx, to force ICD loading on Windows
        //otherwise wglCreateContextAttribsARB, which is an extension, will not be availiable
        tmpCtx = DIRECT_CALL_CHK(wglCreateContext)(hdc);
        DIRECT_CALL_CHK(wglMakeCurrent)(hdc, tmpCtx);
    }

	{
		try {
			ret = DIRECT_CALL_CHK(wglCreateContextAttribsARB)(hdc, sharedCtx, &newAttribList[0]);
		} catch (const std::runtime_error&) {
			//exception was thrown - wglCreateContextAttribsARB is not avaliable. 
			//do nothing - ret value is still not set, it will be set in standard wrapper function
		}    
	}
	

    if (!anyContextPresent) {
        DIRECT_CALL_CHK(wglMakeCurrent)(NULL, NULL);
        DIRECT_CALL_CHK(wglDeleteContext)(tmpCtx);
        anyContextPresent = true;
    }
    return ret;
}

void TextureTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (g_DGLGLState.getCurrent()) {

        if (entrp == glGenTextures_Call || entrp == glGenTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->ensureTexture(names[i]);
            }
        } else if (entrp == glDeleteTextures_Call || entrp == glDeleteTexturesEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->deleteTexture(names[i]);
            }
        } else if (entrp == glBindTexture_Call || entrp == glBindTextureEXT_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            g_DGLGLState.getCurrent()->ensureTexture(name)->setTarget(target);
        }
    }
    PrevPost(call, ret);
}

void BufferTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (g_DGLGLState.getCurrent()) {

        if (entrp == glGenBuffers_Call || entrp ==  glGenBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->ensureBuffer(names[i]);
            }
        } else if (entrp == glDeleteBuffers_Call || entrp == glDeleteBuffersARB_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->deleteBuffer(names[i]);
            }
        } else if (entrp == glBindBuffer_Call || entrp == glBindBufferARB_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                g_DGLGLState.getCurrent()->ensureBuffer(name)->setTarget(target);
            }
        }
    }
    PrevPost(call, ret);
}

RetValue ProgramTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = PrevPre(call);

    if (call.getEntrypoint() == glUseProgram_Call || call.getEntrypoint() == glUseProgramObjectARB_Call) {

        GLint currentProgramName;
        DIRECT_CALL(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProgramName);
        
        if (currentProgramName) {
            dglState::GLProgramObj* currentProgram = g_DGLGLState.getCurrent()->ensureProgram(currentProgramName);

            currentProgram->use(false);
            if (currentProgram->mayDelete()) {
                g_DGLGLState.getCurrent()->deleteProgram(currentProgramName);
            }
        }
    }
    return ret;
}

void ProgramTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (g_DGLGLState.getCurrent()) {
        GLuint name;
        if (entrp == glCreateProgram_Call || entrp == glCreateProgramObjectARB_Call) {

            ret.get(name);

            g_DGLGLState.getCurrent()->ensureProgram(name);

        } else if (entrp == glDeleteProgram_Call || entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLProgramObj* program = g_DGLGLState.getCurrent()->ensureProgram(name);
            program->markDeleted();
            if (program->mayDelete()) {
                g_DGLGLState.getCurrent()->deleteProgram(name);
            }

        } else if (entrp == glUseProgram_Call || entrp == glUseProgramObjectARB_Call) {

            call.getArgs()[0].get(name);

            if (name != 0) {
                g_DGLGLState.getCurrent()->ensureProgram(name)->use(true);
            }
        } else if (entrp == glLinkProgram_Call) {

            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetProgramiv)(name, GL_LINK_STATUS, &linkStatus);

            if (g_Config.m_BreakOnCompilerError &&  linkStatus != GL_TRUE) {
                g_Controller->getBreakState().breakAtCompilerError();
            }

        } else if (entrp == glLinkProgramARB_Call) {
            call.getArgs()[0].get(name);

            GLint linkStatus;
            DIRECT_CALL_CHK(glGetObjectParameterivARB)(name, GL_OBJECT_LINK_STATUS_ARB, &linkStatus);

            if (g_Config.m_BreakOnCompilerError &&  linkStatus != GL_TRUE) {
                g_Controller->getBreakState().breakAtCompilerError();
            }

        }
    }
    PrevPost(call, ret);
}

void ShaderTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();

    if (g_DGLGLState.getCurrent()) {
        GLuint name;
        if (entrp == glCreateShader_Call || entrp == glCreateShaderObjectARB_Call) {

            //we assume that GLhandleARB is the same type as GLuint

            ret.get(name);

            GLenumWrap target;
            call.getArgs()[0].get(target);

            g_DGLGLState.getCurrent()->ensureShader(name)->setTarget(target);

        } else if (entrp == glDeleteShader_Call || entrp == glDeleteObjectARB_Call) {

            call.getArgs()[0].get(name);

            dglState::GLShaderObj* shader = g_DGLGLState.getCurrent()->ensureShader(name);
            shader->markDeleted();

        } else if (entrp == glShaderSource_Call || entrp == glShaderSourceARB_Call) {

            // we assume that GLcharARB is the same as GLchar

            GLsizei  count;
            const GLchar * const* string;
            const GLint* length;
            call.getArgs()[0].get(name);
            call.getArgs()[1].get(count);
            call.getArgs()[2].get(string);
            call.getArgs()[3].get(length);

            std::vector<std::string> sources(count);
            for (size_t i = 0; i < sources.size(); i++) {
                if (length && length[i] > 0) {
                    sources[i] = std::string(string[0], length[i]);
                } else {
                    sources[i] = std::string(string[0]);
                }
            }

            g_DGLGLState.getCurrent()->ensureShader(name)->setSources(sources);


        } else if (entrp == glCompileShader_Call) {
            
            call.getArgs()[0].get(name);

            GLint infoLogLength, compileStatus;
            DIRECT_CALL_CHK(glGetShaderiv)(name, GL_INFO_LOG_LENGTH, &infoLogLength);
            DIRECT_CALL_CHK(glGetShaderiv)(name, GL_COMPILE_STATUS, &compileStatus);

            std::string infoLog; infoLog.resize(infoLogLength);
            GLsizei actualLength;
            DIRECT_CALL_CHK(glGetShaderInfoLog)(name, infoLog.size(), &actualLength, &infoLog[0]);

            if (actualLength < infoLogLength) {
                //highly unlikely - only on buggy drivers
                infoLog.resize(actualLength);
            }

            g_DGLGLState.getCurrent()->ensureShader(name)->setCompilationStatus(infoLog, compileStatus);

            if (g_Config.m_BreakOnCompilerError &&  compileStatus != GL_TRUE) {
                g_Controller->getBreakState().breakAtCompilerError();
            }

        } else if (entrp == glCompileShaderARB_Call) {

            call.getArgs()[0].get(name);

            GLint infoLogLength, compileStatus;
            DIRECT_CALL_CHK(glGetObjectParameterivARB)(name, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
            DIRECT_CALL_CHK(glGetObjectParameterivARB)(name, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);

            std::string infoLog; infoLog.resize(infoLogLength);
            GLsizei actualLength;
            DIRECT_CALL_CHK(glGetInfoLogARB)(name, infoLog.size(), &actualLength, &infoLog[0]);

            if (actualLength < infoLogLength) {
                //highly unlikely - only on buggy drivers
                infoLog.resize(actualLength);
            }

            g_DGLGLState.getCurrent()->ensureShader(name)->setCompilationStatus(infoLog, compileStatus);

            if (g_Config.m_BreakOnCompilerError &&  compileStatus != GL_TRUE) {
                g_Controller->getBreakState().breakAtCompilerError();
            }

        } else if (entrp == glAttachShader_Call || entrp == glAttachObjectARB_Call) {
            GLuint prog, shad;
            call.getArgs()[0].get(prog);
            call.getArgs()[1].get(shad);
            g_DGLGLState.getCurrent()->ensureProgram(prog)->attachShader(g_DGLGLState.getCurrent()->ensureShader(shad));
        }
    }
    PrevPost(call, ret);
}

void ImmediateModeTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    if (g_DGLGLState.getCurrent()) {
        switch (call.getEntrypoint()) {
            case glBegin_Call:
                g_DGLGLState.getCurrent()->setImmediateMode(true);
                break;
            case glEnd_Call:
                g_DGLGLState.getCurrent()->setImmediateMode(false);
                break;
        }
    }
    PrevPost(call, ret);
}


void FBOTracer::Post(const CalledEntryPoint& call, const RetValue& ret) {
    Entrypoint entrp = call.getEntrypoint();
    if (g_DGLGLState.getCurrent()) {

        if (entrp == glGenFramebuffers_Call || entrp == glGenFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->ensureFBO(names[i]);
            }
        } else if (entrp == glDeleteFramebuffers_Call || entrp == glDeleteFramebuffersEXT_Call) {
            GLsizei n = 0;
            call.getArgs()[0].get(n);

            const GLuint* names;
            call.getArgs()[1].get(names);

            for (GLsizei i = 0; i < n; i++) {
                g_DGLGLState.getCurrent()->deleteFBO(names[i]);
            }
        } else if (entrp == glBindFramebuffer_Call || entrp == glBindFramebufferEXT_Call) {
            GLenumWrap target;
            call.getArgs()[0].get(target);
            GLuint name;
            call.getArgs()[1].get(name);
            if (name) {
                g_DGLGLState.getCurrent()->ensureFBO(name)->setTarget(target);
            }
        }
    }
    PrevPost(call, ret);
}