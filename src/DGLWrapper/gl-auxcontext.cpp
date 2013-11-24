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

#include "gl-auxcontext.h"
#include "gl-state.h"
#include "display.h"
#include "native-surface.h"
#include "pointers.h"

#include <sstream>

namespace dglState {

GLAuxContextSession::GLAuxContextSession(GLAuxContext* ctx) : m_ctx(ctx) {
    m_ctx->doRefCurrent();
    m_ctx->queries.setupInitialState();
}

GLAuxContextSession::~GLAuxContextSession() { m_ctx->doUnrefCurrent(); }

GLAuxContextSurface::GLAuxContextSurface(const DGLDisplayState* display,
                                         opaque_id_t pixfmt)
        : m_DisplayId(display->getId()), m_Id(0) {
    EGLint attributes[] = {EGL_HEIGHT, 1, EGL_WIDTH, 1, EGL_NONE};
    m_Id = (opaque_id_t)DIRECT_CALL_CHK(eglCreatePbufferSurface)(
            (EGLDisplay)m_DisplayId, (EGLConfig)pixfmt, attributes);
    if (!m_Id) {
        throw std::runtime_error("Cannot allocate axualiary pbuffer surface");
    }
}

GLAuxContextSurface::~GLAuxContextSurface() {
    if (m_Id) {
        DIRECT_CALL_CHK(eglDestroySurface)((EGLDisplay)m_DisplayId,
                                           (EGLSurface)m_Id);
    }
}

opaque_id_t GLAuxContextSurface::getId() const { return m_Id; }

GLAuxContext::GLAuxContext(const GLContext* origCtx)
        : queries(this),
          m_Id(0),
          m_PixelFormat(0),
          m_Parrent(origCtx),
          m_MakeCurrentRef(0) {

    if (m_Parrent->getDisplay()->getType() != DGLDisplayState::Type::EGL) {
        throw std::runtime_error("auxaliary contexts implemented only for EGL");
    }

    std::vector<EGLint> eglAttributes(
            m_Parrent->getContextCreationData().getAttribs().size());

    for (size_t i = 0;
         i < m_Parrent->getContextCreationData().getAttribs().size(); i++) {
        eglAttributes[i] =
                (EGLint)m_Parrent->getContextCreationData().getAttribs()[i];
    }
    eglAttributes.push_back(EGL_NONE);

    m_PixelFormat = choosePixelFormat(
            m_Parrent->getContextCreationData().getPixelFormat(),
            m_Parrent->getDisplay()->getId());

    switch (origCtx->getContextCreationData().getEntryPoint()) {
        case eglCreateContext_Call:
            m_Id = (opaque_id_t)DIRECT_CALL_CHK(eglCreateContext)(
                    (EGLDisplay)m_Parrent->getDisplay()->getId(),
                    (EGLConfig)m_PixelFormat, (EGLContext)m_Parrent->getId(),
                    &eglAttributes[0]);
            break;
    }

    if (!m_Id) {
        throw std::runtime_error("Cannot allocate auxaliary context");
    }
}

GLAuxContext::~GLAuxContext() {

    while (m_MakeCurrentRef) {
        doUnrefCurrent();
    }

    if (m_Id) {
        DIRECT_CALL_CHK(eglDestroyContext)(
                (EGLDisplay)m_Parrent->getDisplay()->getId(), (EGLContext)m_Id);
    }
}

GLAuxContextSession GLAuxContext::makeCurrent() {
    return GLAuxContextSession(this);
}

void GLAuxContext::doRefCurrent() {

    if (!m_MakeCurrentRef) {
        if (!m_AuxSurface) {
            m_AuxSurface = std::make_shared<GLAuxContextSurface>(
                    m_Parrent->getDisplay(), m_PixelFormat);
        }
        EGLBoolean status = DIRECT_CALL_CHK(eglMakeCurrent)(
                (EGLDisplay)m_Parrent->getDisplay()->getId(),
                (EGLSurface)m_AuxSurface->getId(),
                (EGLSurface)m_AuxSurface->getId(), (EGLContext)m_Id);

        if (!status) {
            throw std::runtime_error("Cannot switch to auxaliary context.");
        }
    }

    m_MakeCurrentRef++;
}

void GLAuxContext::doUnrefCurrent() {
    if (m_MakeCurrentRef) {
        m_MakeCurrentRef--;
    }
    if (!m_MakeCurrentRef) {
        EGLBoolean status = DIRECT_CALL_CHK(eglMakeCurrent)(
                (EGLDisplay)m_Parrent->getDisplay()->getId(),
                (EGLSurface)m_Parrent->getNativeDrawSurface()->getId(),
                (EGLSurface)m_Parrent->getNativeReadSurface()->getId(),
                (EGLContext)m_Parrent->getId());
        if (!status) {
            throw std::runtime_error(
                    "Cannot switch back from auxaliary context.");
        }
    }
}

opaque_id_t GLAuxContext::choosePixelFormat(opaque_id_t preferred,
                                            opaque_id_t displayId) {
    EGLDisplay eglDpy = (EGLDisplay)displayId;
    EGLConfig preferredConfig = (EGLConfig)preferred;
    EGLint supportedSurfType = 0;

    EGLBoolean status = EGL_TRUE;
    status &= DIRECT_CALL_CHK(eglGetConfigAttrib)(
            eglDpy, preferredConfig, EGL_SURFACE_TYPE, &supportedSurfType);

    if (!status) {
        throw std::runtime_error("Cannot query EGLConfig associated with ctx");
    }

    EGLConfig ret;

    if (supportedSurfType & EGL_PBUFFER_BIT) {
        ret = preferredConfig;
    } else {

        EGLint renderableType = 0;

        if (m_Parrent->getVersion().check(GLContextVersion::Type::DT)) {
            renderableType = EGL_OPENGL_BIT;
        } else if (m_Parrent->getVersion().check(GLContextVersion::Type::ES,
                                                 3)) {
            renderableType = EGL_OPENGL_ES3_BIT_KHR;
        } else if (m_Parrent->getVersion().check(GLContextVersion::Type::ES,
                                                 2)) {
            renderableType = EGL_OPENGL_ES2_BIT;
        } else if (m_Parrent->getVersion().check(GLContextVersion::Type::ES,
                                                 1)) {
            renderableType = EGL_OPENGL_ES_BIT;
        }

        EGLint attributes[] = {EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
                               EGL_RENDERABLE_TYPE, renderableType,
                               EGL_NONE};
        EGLint numConfigs = 0;
        status &= DIRECT_CALL_CHK(eglChooseConfig)(eglDpy, attributes, &ret, 1,
                                                   &numConfigs);
        if ((status != EGL_TRUE) || numConfigs < 1) {
            throw std::runtime_error(
                    "Cannot choose EGLConfig capable of driving auxaliary "
                    "context");
        }
    }
    return (opaque_id_t)ret;
}

GLAuxContext::GLQueries::GLQueries(GLAuxContext* ctx)
        : m_InitialState(false), m_AuxCtx(ctx) {}

void GLAuxContext::GLQueries::setupInitialState() {

    if (m_InitialState) return;
    DIRECT_CALL_CHK(glGenFramebuffers)(1, &fbo);
    DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, fbo);

    DIRECT_CALL_CHK(glGenRenderbuffers)(1, &rbo);
    DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, rbo);
    DIRECT_CALL_CHK(glFramebufferRenderbuffer)(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    if (m_AuxCtx->m_Parrent->getVersion().check(GLContextVersion::Type::ES,
                                                3) ||
        m_AuxCtx->m_Parrent->getVersion().check(GLContextVersion::Type::DT,
                                                3)) {
        DIRECT_CALL_CHK(glGenVertexArrays)(1, &vao);
        DIRECT_CALL_CHK(glBindVertexArray)(vao);
    }

    DIRECT_CALL_CHK(glGenBuffers)(1, &vbo);
    DIRECT_CALL_CHK(glBindBuffer)(GL_ARRAY_BUFFER, vbo);
    GLfloat triangleStrip[] = {-1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f,
                               0.0f,  1.0f,  1.0f, 1.0f, 0.0f,  1.0f,
                               1.0f,  -1.0f, 0.0f, 1.0f};
    DIRECT_CALL_CHK(glBufferData)(GL_ARRAY_BUFFER, sizeof(triangleStrip),
                                  triangleStrip, GL_STATIC_DRAW);
    DIRECT_CALL_CHK(glVertexAttribPointer)(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    DIRECT_CALL_CHK(glEnableVertexAttribArray)(0);

    vshobj = DIRECT_CALL_CHK(glCreateShader)(GL_VERTEX_SHADER);
    const char* vsh =
            "attribute vec4 inPos;\n"
            "varying vec2 texPos;\n"
            "void main() {\n"
            "   gl_Position = inPos;\n"
            "   texPos = inPos.xy * 0.5 + 0.5;\n"
            "}\n";

    DIRECT_CALL_CHK(glShaderSource)(vshobj, 1, &vsh, NULL);
    DIRECT_CALL_CHK(glCompileShader)(vshobj);
    GLint status = 0;
    DIRECT_CALL_CHK(glGetShaderiv)(vshobj, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1000];
        DIRECT_CALL_CHK(glGetShaderInfoLog)(vshobj, 1000, NULL, log);
        throw std::runtime_error(std::string("Cannot compile vertex shader") +
                                 log);
    }

    if (DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR) {
        throw std::runtime_error("Got GL error on auxiliary context");
    }

    m_InitialState = true;
}

void GLAuxContext::GLQueries::auxDrawTexture(GLuint name, GLenum target,
                                             GLint level,
                                             GLenum textureBaseFormat,
                                             GLenum renderableFormat, int width,
                                             int height) {

    if (!m_AuxCtx->m_Parrent->getVersion().check(GLContextVersion::Type::ES,
                                                 2)) {
        throw std::runtime_error(
                "GLAuxContext::GLQueries::auxGetTexImage not implemented for "
                "this "
                "context version");
    }

    if (!DIRECT_CALL_CHK(glIsTexture)(name)) {
        throw std::runtime_error(
                "Texture object not found in auxaliary context");
    }

    DIRECT_CALL_CHK(glBindTexture)(target, name);

    DIRECT_CALL_CHK(glRenderbufferStorage)(GL_RENDERBUFFER, renderableFormat, width,
                                           height);

    GLuint program = getTextureShaderProgram(target, textureBaseFormat);
    DIRECT_CALL_CHK(glUseProgram)(program);
    DIRECT_CALL_CHK(glUniform1f)(
            DIRECT_CALL_CHK(glGetUniformLocation)(program, "level"),
            static_cast<GLfloat>(level));

    DIRECT_CALL_CHK(glDrawArrays)(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint GLAuxContext::GLQueries::getTextureShaderProgram(
        GLenum target, GLenum textureBaseFormat) {

    std::string suffix, pos;

    if (textureBaseFormat != GL_RGBA && textureBaseFormat != GL_RGB &&
        textureBaseFormat != GL_RG && textureBaseFormat != GL_RED) {
        throw std::runtime_error(
                "GLAuxContext::GLQueries::getTextureShaderProgram: format "
                "unsupported");
    }

    switch (target) {
        case GL_TEXTURE_1D:
            suffix = "1D";
            pos = "texPos.x";
            break;
        case GL_TEXTURE_2D:
            suffix = "2D";
            pos = "vec2(texPos.xy)";
            break;
        case GL_TEXTURE_RECTANGLE:
            suffix = "Rect";
            pos = "vec2(texPos.xy)";
            break;
        case GL_TEXTURE_1D_ARRAY:
            suffix = "1DArray";
            pos = "vec2(texPos.xy)";
            break;
        // case GL_TEXTURE_CUBE_MAP:
        //  suffix = "Cube";
        //  pos =
        //  break;
        default:
            throw std::runtime_error(
                    "cannot generate program for this texture target");
    }

    bool glsl300 = m_AuxCtx->m_Parrent->getVersion().check(
            GLContextVersion::Type::ES, 3);

    std::ostringstream fsh;

    if (glsl300) {
        fsh << "#version 300 es\n"
            << "out vec4 oColor\n;"
            << "varying vec2 texPos;\n";
    }
    fsh << "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
        << "precision highp float;           \n"
        << "#else                            \n"
        << "precision mediump float;         \n"
        << "#endif                           \n"
           "uniform sampler" << suffix << " s;\n"
                                          "uniform float level;\n"
        << "varying vec2 texPos;\n";

    fsh << "void main() {\n";

    if (glsl300) {
        fsh << "    oColor = textureLod(s, ";
    } else {
        fsh << "    gl_FragColor = texture" << suffix << "(s, ";
    }
    fsh << pos;
    if (glsl300) {
        fsh << ", level";
    }
    fsh << ");\n"
        << "}\n";

    auto i = programs.find(fsh.str());
    if (i != programs.end()) {
        return i->second;
    } else {

        GLuint program = DIRECT_CALL_CHK(glCreateProgram)();
        DIRECT_CALL_CHK(glAttachShader)(program, vshobj);

        GLuint fShader = DIRECT_CALL_CHK(glCreateShader)(GL_FRAGMENT_SHADER);
        DIRECT_CALL_CHK(glAttachShader)(program, fShader);
        DIRECT_CALL_CHK(glDeleteShader)(fShader);

        std::string fshStr = fsh.str();
        const char* csrc[] = {fshStr.c_str()};

        DIRECT_CALL_CHK(glShaderSource)(fShader, 1, csrc, NULL);
        DIRECT_CALL_CHK(glCompileShader)(fShader);

        GLint status = 0;
        DIRECT_CALL_CHK(glGetShaderiv)(fShader, GL_COMPILE_STATUS, &status);
        if (!status) {
            char log[1000];
            DIRECT_CALL_CHK(glGetShaderInfoLog)(fShader, 1000, NULL, log);
            throw std::runtime_error(
                    std::string("Cannot compile fragment shader:") + log);
        }

        DIRECT_CALL_CHK(glLinkProgram)(program);

        DIRECT_CALL_CHK(glGetProgramiv)(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            char log[10000];
            DIRECT_CALL_CHK(glGetProgramInfoLog)(program, 10000, NULL, log);
            DIRECT_CALL_CHK(glDeleteProgram)(program);
            throw std::runtime_error(std::string("cannot link program: ") +
                                     log);
        } else {
            programs[fshStr] = program;
            return program;
        }
    }
}

}    // namespace dglState