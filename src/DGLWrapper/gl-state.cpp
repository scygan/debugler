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

#include "gl-state.h"

#ifdef HAVE_LIBRARY_GLX
//for GLXBadDrawable:
#include <X11/Xproto.h>
#include <GL/glxproto.h>
#endif


#include <boost/make_shared.hpp>
#include <string>
#include <cstring>
#include <stdexcept>
#include <sstream>

#include "pointers.h"
#include "api-loader.h"
#include "tls.h"
#include "native-surface.h"
#include "gl-utils.h"

#include <DGLCommon/def.h>
#include <DGLNet/protocol/pixeltransfer.h>


namespace dglState {

namespace state_setters {

    class DefaultPBO {
    public:
        DefaultPBO() {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &m_PBO);
            if (m_PBO) {
                DIRECT_CALL_CHK(glBindBuffer)(GL_PIXEL_PACK_BUFFER, 0);
            }
        }
        ~DefaultPBO() {
            if (m_PBO) {
                DIRECT_CALL_CHK(glBindBuffer)(GL_PIXEL_PACK_BUFFER, m_PBO);
            }
        }
    private:
        GLint m_PBO;
    };

    class CurrentFramebuffer {
    public:
        CurrentFramebuffer(GLuint name) {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_FRAMEBUFFER_BINDING, &m_ReadFBO);
            DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_FRAMEBUFFER_BINDING, &m_DrawFBO);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_FRAMEBUFFER, name);
        }
        ~CurrentFramebuffer() {
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, m_ReadFBO);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_DRAW_FRAMEBUFFER, m_DrawFBO);
        }
    private:
        GLint m_ReadFBO, m_DrawFBO;
    };

    class ReadBuffer {
    public:
        ReadBuffer() {
            if (gc->getVersion().check(GLContextVersion::ES, 3) || gc->getVersion().check(GLContextVersion::DT)) {
                DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &m_ReadBuffer);
            }
        }
        ~ReadBuffer() {
            if (gc->getVersion().check(GLContextVersion::ES, 3) || gc->getVersion().check(GLContextVersion::DT)) {
                DIRECT_CALL_CHK(glReadBuffer)(m_ReadBuffer);
            }
        }
    private:
        GLint m_ReadBuffer;
    };

    class DrawBuffers {
    public:
        DrawBuffers() {
            GLint maxDrawBuffers;
            DIRECT_CALL_CHK(glGetIntegerv)(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
            m_DrawBuffers.resize(maxDrawBuffers);
            for (GLint i = 0; i < maxDrawBuffers; i++) {
                DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_BUFFER0 + i, &m_DrawBuffers[i]);
            }
        }
        ~DrawBuffers() {
            DIRECT_CALL_CHK(glDrawBuffers)(static_cast<GLsizei>(m_DrawBuffers.size()), reinterpret_cast<GLenum*>(&m_DrawBuffers[0]));
        }
    private:
        std::vector<GLint> m_DrawBuffers;        
    };

    class RenderBuffer {
    public:
        RenderBuffer() {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_RENDERBUFFER_BINDING, &m_RenderBuffer);
        }
        ~RenderBuffer() {
            DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, m_RenderBuffer);
        }
    private:
        GLint m_RenderBuffer;
    };

    class PixelStoreAlignment {
#define STATE_SIZE 8
    public:
        PixelStoreAlignment() {
            //dump and set pixel store state
            for (int i = 0; i < STATE_SIZE; i++) {
                if (gc->getVersion().check(GLContextVersion::DT) ||
                        (s_StateTable[i].m_ES3 && gc->getVersion().check(GLContextVersion::ES, 3))) {
                    DIRECT_CALL_CHK(glGetIntegerv)(s_StateTable[i].m_Target, &s_StateTable[i].m_SavedState);
                    DIRECT_CALL_CHK(glPixelStorei)(s_StateTable[i].m_Target, s_StateTable[i].m_State);
                }
            }
        }
        ~PixelStoreAlignment() {
            for (int i = 0; i < STATE_SIZE; i++) {
                if (gc->getVersion().check(GLContextVersion::DT) ||
                        (s_StateTable[i].m_ES3 && gc->getVersion().check(GLContextVersion::ES, 3))) {
                    DIRECT_CALL_CHK(glPixelStorei)(s_StateTable[i].m_Target, s_StateTable[i].m_SavedState);
                }
            }
        }
    public:
        int getAligned(int x) {
            int a = s_StateTable[7].m_State;
            return (x + a - 1) & (-a);
        }
    private:

        static struct StateEntry {
            GLenum m_Target;
            GLint m_State;
            bool m_ES3;
            GLint m_SavedState;
        } s_StateTable[STATE_SIZE];
    };
    
    PixelStoreAlignment::StateEntry PixelStoreAlignment::s_StateTable[STATE_SIZE] = {
        { GL_PACK_SWAP_BYTES,   GL_FALSE, false },
        { GL_PACK_LSB_FIRST,    GL_FALSE, false },
        { GL_PACK_ROW_LENGTH,   0,        true },
        { GL_PACK_IMAGE_HEIGHT, 0,        false },
        { GL_PACK_SKIP_ROWS,    0,        true },
        { GL_PACK_SKIP_PIXELS,  0,        true },
        { GL_PACK_SKIP_IMAGES,  0,        false },
        { GL_PACK_ALIGNMENT,    4,        true },
    };
}
#undef STATE_SIZE


GLObj::GLObj():m_Name(0) {}

GLObj::GLObj(GLuint name):m_Name(name) {}

GLuint GLObj::getName() const { return m_Name; }


GLTextureObj::GLTextureObj(GLuint name):GLObj(name),m_Target(0) {}

void GLTextureObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLTextureObj::getTarget() {
    return m_Target;
}

GLenum GLTextureObj::getTextureLevelTarget(int face) {
    if (m_Target == GL_TEXTURE_CUBE_MAP) {
        GLenum cubeMapFaces[] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };
        assert(face < sizeof(cubeMapFaces)/sizeof(cubeMapFaces[0]));
        return cubeMapFaces[face];
    } else {
        return getTarget();
    }
}

GLBufferObj::GLBufferObj(GLuint name):GLObj(name),m_Target(0) {}

void GLBufferObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLBufferObj::getTarget() {
    return m_Target;
}

GLProgramObj::GLProgramObj(GLuint name):GLObj(name), m_InUse(false) {}

GLProgramObj::~GLProgramObj() {
    auto i = m_AttachedShaders.begin();
    while (i != m_AttachedShaders.end()) {
        detachShader(*(i++));
    }
}

void GLProgramObj::use(bool) {
    m_InUse = true;
}

bool GLProgramObj::mayDelete() {
    return !m_InUse && m_Deleted;
}

void GLProgramObj::markDeleted() {
    m_Deleted = true;
}

void GLProgramObj::attachShader(GLShaderObj* shader) {
    m_AttachedShaders.insert(shader);
    shader->incRefCount();
}

void GLProgramObj::detachShader(GLShaderObj* shader) {
    m_AttachedShaders.erase(shader);
    shader->decRefCount();
}

std::set<GLShaderObj*>& GLProgramObj::getAttachedShaders() {
    return m_AttachedShaders;
}

GLShaderObj::GLShaderObj(GLuint name, bool arbApi):GLObj(name), m_Deleted(false), m_DeleteCalled(false), m_Target(0), m_arbApi(arbApi), m_RefCount(0) {}

void GLShaderObj::deleteCalled() {
    m_DeleteCalled = true;
    mayDelete();
}

void GLShaderObj::incRefCount() {
    m_RefCount++;
}
void GLShaderObj::decRefCount() {
    m_RefCount--;
    mayDelete();
}

int GLShaderObj::getRefCount() {
    return m_RefCount;
}

GLint GLShaderObj::queryCompilationStatus() const {
    if (m_Deleted) {
        return 0;
    }
    GLint compileStatus;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(getName(), GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_COMPILE_STATUS, &compileStatus);
    }
    return compileStatus;
}

std::string GLShaderObj::queryCompilationInfoLog() const {
    if (m_Deleted) {
        return "";
    }

    GLint infoLogLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(getName(), GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_INFO_LOG_LENGTH, &infoLogLength);
    }

    std::vector<GLchar> infoLog(infoLogLength + 1);
    
    GLint actualLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetInfoLogARB)(getName(), static_cast<GLsizei>(infoLog.size()), &actualLength, &infoLog[0]);
    } else {
        DIRECT_CALL_CHK(glGetShaderInfoLog)(getName(), static_cast<GLsizei>(infoLog.size()), &actualLength, &infoLog[0]);
    }
    infoLog[std::min(infoLog.size() - 1, static_cast<size_t>(actualLength))] = 0;

    return &infoLog[0];
}

std::string GLShaderObj::querySources() {
    if (m_Deleted) {
        //shader does not exist in shader memory any more
        return m_LastSources;
    }

    std::vector<GLchar> sources;
    GLint length;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(getName(), GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &length);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_SHADER_SOURCE_LENGTH, &length);
    }
    sources.resize(length + 1);
    GLint actualLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetShaderSourceARB)(getName(), length, &actualLength, &sources[0]);
    } else {
        DIRECT_CALL_CHK(glGetShaderSource)(getName(), length, &actualLength, &sources[0]);
    }
    sources[std::min(sources.size() - 1, static_cast<size_t>(actualLength))] = 0;


    return &sources[0];
}

void GLShaderObj::cacheSources() {
    if (m_Deleted) return;

    m_LastSources = querySources();
}

bool GLShaderObj::isDeleted() const {
    return m_Deleted;
}

void GLShaderObj::editSource(const std::string& source) {
    const char* sourcePtr = source.c_str();
    if (m_arbApi) {
        DIRECT_CALL_CHK(glShaderSourceARB)(getName(), 1, &sourcePtr, NULL);
        DIRECT_CALL_CHK(glCompileShaderARB)(getName());
    } else {
        DIRECT_CALL_CHK(glShaderSource)(getName(), 1, &sourcePtr, NULL);
        DIRECT_CALL_CHK(glCompileShader)(getName());
    } 
}


void GLShaderObj::mayDelete() {
    if (m_RefCount == 0 && m_DeleteCalled) {
        m_Deleted = true;
    }
}

GLenum GLShaderObj::getTarget() const {
    return m_Target;   
}

void GLShaderObj::createCalled(GLenum target) {
    m_Target = target;
    m_Deleted = false;
    m_DeleteCalled = false;
    m_RefCount = 0;
    m_LastSources = "";
}


GLFBObj::GLFBObj(GLuint name):GLObj(name),m_Target(0) {}

void GLFBObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLFBObj::getTarget() {
    return m_Target;
}

GLContextVersion::GLContextVersion(Type type):m_Filled(false), m_Type(type) {}

bool GLContextVersion::check(Type type, int majorVersion, int minorVersion) {

    if (type != m_Type) {
        return false;
    }

    if (!m_Filled)
        fill(); 

    if (majorVersion > m_MajorVersion) {
        return false;
    }

    if (majorVersion == m_MajorVersion && minorVersion > m_MinorVersion) {
        return false;
    }

    return true;
}

void GLContextVersion::fill() {
    //It is safe to assume OpenGL ES 1.1 / OpenGL 1.1 is supported, if version string 
    //parsing fails.
    m_MajorVersion = m_MinorVersion = 1;

    const char* cVersion = reinterpret_cast<const char*>(DIRECT_CALL_CHK(glGetString)(GL_VERSION));

    assert(cVersion);

    if (cVersion == NULL) {
        return;
    }

    std::string version = cVersion;

    int vOffset = -1;
    if (m_Type == DT) {
        vOffset = 0;
    } else if (m_Type == ES) {
        if (version.substr(0, strlen("OpenGL ES ")) == "OpenGL ES ") {
            vOffset = (int)strlen("OpenGL ES ");
        } else if (version.substr(0, strlen("OpenGL ES-")) == "OpenGL ES-") {
            vOffset = (int)strlen("OpenGL ES-");
        } else {
            assert(!"unrecognized GL_VERSION string");
        }
    }

    if (vOffset >= 0 && vOffset + 2 <= (int)version.length()) {

        char buf[] = {0, 0};
        buf[0] = version[vOffset];
        m_MajorVersion = atoi(buf);
        assert(version[vOffset + 1] == '.');
        buf[0] = version[vOffset + 2];
        m_MinorVersion = atoi(buf);
    } else {
        assert(!"cannot reliably detect OpenGL version");
    }

    m_Filled = true;
}


GLContext::GLContext(GLContextVersion version, opaque_id_t id): m_Version(version), m_Id(id), m_NativeReadSurface(NULL), m_HasNVXMemoryInfo(false),
    m_HasDebugOutput(false), m_InImmediateMode(false),m_EverBound(false), m_RefCount(0), m_ToBeDeleted(false)  {}

dglnet::message::BreakedCall::ContextReport GLContext::describe() {
    dglnet::message::BreakedCall::ContextReport ret(m_Id);
    for (std::map<GLuint, GLTextureObj>::iterator i = m_Textures.begin(); 
        i != m_Textures.end(); i++) {
            ret.m_TextureSpace.insert(dglnet::ContextObjectName(m_Id, i->second.getName(), i->second.getTarget()));
    }
    for (std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.begin(); 
        i != m_Buffers.end(); i++) {
            ret.m_BufferSpace.insert(dglnet::ContextObjectName(m_Id, i->second.getName()));
    }
    for (std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.begin(); 
        i != m_Shaders.end(); i++) {
            ret.m_ShaderSpace.insert(dglnet::ContextObjectName(m_Id, i->second.getName(), i->second.getTarget()));
    }
    for (std::map<GLuint, GLProgramObj>::iterator i = m_Programs.begin(); 
        i != m_Programs.end(); i++) {
            ret.m_ProgramSpace.insert(dglnet::ContextObjectName(m_Id, i->second.getName()));
    }
    for (std::map<GLuint, GLFBObj>::iterator i = m_FBOs.begin(); 
        i != m_FBOs.end(); i++) {
            ret.m_FBOSpace.insert(dglnet::ContextObjectName(m_Id, i->second.getName()));
    }
    if (m_NativeReadSurface) {
        if (m_NativeReadSurface->isStereo()) {
            if (m_NativeReadSurface->isDoubleBuffered()) {
                ret.m_FramebufferSpace.insert(dglnet::ContextObjectName(m_Id, GL_BACK_RIGHT));
            }
            ret.m_FramebufferSpace.insert(dglnet::ContextObjectName(m_Id, GL_FRONT_RIGHT));
        }
        if (m_NativeReadSurface->isDoubleBuffered()) {
            ret.m_FramebufferSpace.insert(dglnet::ContextObjectName(m_Id, GL_BACK));
        }
        ret.m_FramebufferSpace.insert(dglnet::ContextObjectName(m_Id, GL_FRONT));
    }
    return ret;
}

NativeSurfaceBase* GLContext::getNativeReadSurface() {
    return m_NativeReadSurface;
}

void GLContext::setNativeReadSurface(NativeSurfaceBase* surface) {
    m_NativeReadSurface = surface;
}

GLTextureObj* GLContext::ensureTexture(GLuint name) {
    std::map<GLuint, GLTextureObj>::iterator i = m_Textures.find(name);
    if (i == m_Textures.end()) {
        i = m_Textures.insert(std::pair<GLuint, GLTextureObj>(name, GLTextureObj(name))).first;
    }
    return &(*i).second;
}

void GLContext::deleteTexture(GLuint name) {
    std::map<GLuint, GLTextureObj>::iterator i = m_Textures.find(name); 
    if (i !=  m_Textures.end()) {
        m_Textures.erase(i);
    }
}


GLBufferObj* GLContext::ensureBuffer(GLuint name) {
    std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.find(name);
    if (i == m_Buffers.end()) {
        i = m_Buffers.insert(std::pair<GLuint, GLBufferObj>(name, GLBufferObj(name))).first;
    }
    return &(*i).second;
}

void GLContext::deleteBuffer(GLuint name) {
    std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.find(name); 
    if (i !=  m_Buffers.end()) {
        m_Buffers.erase(i);
    }
}

GLFBObj* GLContext::ensureFBO(GLuint name) {
    std::map<GLuint, GLFBObj>::iterator i = m_FBOs.find(name);
    if (i == m_FBOs.end()) {
        i = m_FBOs.insert(std::pair<GLuint, GLFBObj>(name, GLFBObj(name))).first;
    }
    return &(*i).second;
}

void GLContext::deleteFBO(GLuint name) {
    std::map<GLuint, GLFBObj>::iterator i = m_FBOs.find(name); 
    if (i !=  m_FBOs.end()) {
        m_FBOs.erase(i);
    }
}

std::pair<bool, GLenum> GLContext::getPokedError() {
    std::pair<bool, GLenum> ret;
    if (m_PokedErrorQueue.size()) {
        ret.first = true;
        ret.second = m_PokedErrorQueue.front();
        m_PokedErrorQueue.pop();
    } else {
        ret.first = false;
    }
    return ret;
}

GLenum GLContext::peekError() {

    if (m_InImmediateMode) return GL_NO_ERROR; //we cannot get erros after glBegin()

    GLenum ret = DIRECT_CALL_CHK(glGetError)();
    if (ret != GL_NO_ERROR && m_PokedErrorQueue.size() < 1000) {
        GLenum error = ret;
        int retries = 16;
        do {
            m_PokedErrorQueue.push(error);
            error = DIRECT_CALL_CHK(glGetError)();
            retries --;
        } while (error != GL_NO_ERROR && retries);
    }
    return ret;
}

void GLContext::setDebugOutput(const std::string& message) {
    m_HasDebugOutput = true; 
    m_DebugOutput = message;
}

bool GLContext::hasDebugOutput() {
    return m_HasDebugOutput;
}

const std::string& GLContext::popDebugOutput() {
    m_HasDebugOutput = false;
    return m_DebugOutput;
}

void GLContext::startQuery() {

    if (m_Version.check(GLContextVersion::UNSUPPORTED)) {
        throw std::runtime_error("Context version is not supported");
    }

    peekError();
    if (m_InImmediateMode) {
        throw std::runtime_error("OpenGL is currently in immediate mode (after glBegin,  before glEnd) - cannot issue query");
    }
}

void GLContext::queryCheckError() {
    GLenum error;
    if  ((error = DIRECT_CALL_CHK(glGetError)()) != GL_NO_ERROR ) {
        throw std::runtime_error(std::string("Query failed: got OpenGL error (") + GetGLEnumName(error) + ")");
    }
}

bool GLContext::endQuery(std::string& message) {
    bool ret = true;
    GLenum error;
    if  (!m_InImmediateMode && (error = DIRECT_CALL_CHK(glGetError)()) != GL_NO_ERROR ) {
        message = std::string("Query failed: got OpenGL error (") + GetGLEnumName(error) + ")";
        ret = false;
    }
    while (!m_InImmediateMode && DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR);
    
    //always invalidate debug output from query functions
    m_HasDebugOutput = false;

    return ret;
}

void GLContext::setImmediateMode(bool immed) {
    m_InImmediateMode = immed;
}

void GLContext::bound() {
    if (!m_EverBound) {
        m_EverBound = true;
        firstUse();
    }
    m_RefCount++;
}

 bool GLContext::markForDeletionMayDelete() {
     m_ToBeDeleted = true;
     return m_RefCount <= 0;
 }

bool GLContext::unboundMayDelete() {
    m_RefCount--;
    assert(m_RefCount >= 0);
    return m_ToBeDeleted && m_RefCount <= 0;
}

GLContextVersion& GLContext::getVersion() {
    return m_Version;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryTexture(gl_t _name) {

    GLuint name = static_cast<GLuint>(_name);

    dglnet::resource::DGLResourceTexture* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceTexture());

    //check if GL knows about a texture
    if (DIRECT_CALL_CHK(glIsTexture)(name) != GL_TRUE) {
        throw std::runtime_error("Texture does not exist");
    }

    //check if we know about a texture target
    GLTextureObj* tex = ensureTexture(name);
    if (tex->getTarget() == 0) {
        throw std::runtime_error("Texture target is unknown");
    } else if (tex->getTarget() != GL_TEXTURE_1D &&
               tex->getTarget() != GL_TEXTURE_2D &&
               tex->getTarget() != GL_TEXTURE_RECTANGLE &&
               tex->getTarget() != GL_TEXTURE_1D_ARRAY && 
               tex->getTarget() != GL_TEXTURE_CUBE_MAP) {
        throw std::runtime_error("Texture target is unsupported");
    }

    //disconnect PBO if it exists
    state_setters::DefaultPBO defPBO;
    state_setters::PixelStoreAlignment defAlignment;

    //rebind texture, so we can access it
    GLuint lastTexture = glutils::getBoundTexture(tex->getTarget());
    if (lastTexture != tex->getName()) {
        DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), tex->getName());
    }

    if (tex->getTarget() == GL_TEXTURE_CUBE_MAP) {
        resource->m_FacesLevels.resize(6);
    } else {
        resource->m_FacesLevels.resize(1);
    }

    for (size_t face = 0; face < resource->m_FacesLevels.size(); face++) {
        for (int level = 0;; level++) {

            GLint height, width, samples = 0;

            GLenum levelTarget = tex->getTextureLevelTarget((int)face);

            GLint internalFormat;
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_WIDTH, &width);
            if (tex->getTarget() == GL_TEXTURE_1D) {
                height = 1;
            } else {
                DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_HEIGHT, &height);
            }
            if (!width || !height || DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR)
                break;
         
            std::vector<GLint> rgbaSizes(4, 0);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_RED_SIZE, &rgbaSizes[0]);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_GREEN_SIZE, &rgbaSizes[1]);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_BLUE_SIZE, &rgbaSizes[2]);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_ALPHA_SIZE, &rgbaSizes[3]);

            if (gc->getVersion().check(GLContextVersion::DT, 3, 2)) {
                DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_SAMPLES, &samples);
            }

            std::vector<GLint> deptStencilSizes(2, 0);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_DEPTH_SIZE, &deptStencilSizes[0]);
            
            if (gc->getVersion().check(GLContextVersion::DT, 3)) { 
                DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level, GL_TEXTURE_STENCIL_SIZE, &deptStencilSizes[1]);
            }

            queryCheckError();

            DGLPixelTransfer transfer(rgbaSizes, deptStencilSizes, internalFormat);

            resource->m_FacesLevels[face].push_back(boost::make_shared<dglnet::resource::DGLPixelRectangle>(width, height, defAlignment.getAligned(width * transfer.getPixelSize()),
                transfer.getFormat(), transfer.getType(), internalFormat, samples));
            
            GLvoid* ptr;
            if ((ptr = resource->m_FacesLevels[face].back()->getPtr()) != NULL) {
                DIRECT_CALL_CHK(glGetTexImage)(levelTarget, level, transfer.getFormat(), transfer.getType(), ptr);
            }
        }
    }

    //restore state
    if (lastTexture != tex->getName()) {
        DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), lastTexture);
    }
    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryBuffer(gl_t _name) {

    GLuint name = static_cast<GLuint>(_name);

    dglnet::resource::DGLResourceBuffer* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceBuffer());

    //check if GL knows about a texture
    if (DIRECT_CALL_CHK(glIsBuffer)(name) != GL_TRUE) {
        throw std::runtime_error("Buffer does not exist");
    }

    //check if we know about a texture target
    GLBufferObj* buff = ensureBuffer(name);
    if (buff->getTarget() == 0) {
        throw std::runtime_error("Buffer target is unknown");
    }

    //rebind buffer, so we can access it
    GLint i;
    switch (buff->getTarget()) {
        case GL_ARRAY_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, &i);
            break;
        case GL_ATOMIC_COUNTER_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_ATOMIC_COUNTER_BUFFER_BINDING, &i);
            break;
        case GL_COPY_READ_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_COPY_READ_BUFFER_BINDING, &i);
            break;
        case GL_COPY_WRITE_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_COPY_WRITE_BUFFER_BINDING, &i);
            break;
        case GL_DRAW_INDIRECT_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_INDIRECT_BUFFER_BINDING, &i);
            break;
        case GL_DISPATCH_INDIRECT_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &i);
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &i);
            break;
        case GL_PIXEL_PACK_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &i);
            break;
        case GL_PIXEL_UNPACK_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)( GL_PIXEL_UNPACK_BUFFER_BINDING, &i);
            break;
        case GL_SHADER_STORAGE_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_SHADER_STORAGE_BUFFER_BINDING, &i);
            break;
        /*case GL_TEXTURE_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_NO_IDEA_WHAT_SHOULD_BE_HERE, &lastBuffer);
            break;*/
        case GL_TRANSFORM_FEEDBACK_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &i);
            break;
        case GL_UNIFORM_BUFFER:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_UNIFORM_BUFFER_BINDING, &i);
            break;       
    default:
        throw std::runtime_error("Buffer target is not supported");
    }
    GLuint lastBuffer = static_cast<GLuint>(i);
    if (lastBuffer != buff->getName()) {
        DIRECT_CALL_CHK(glBindBuffer)(buff->getTarget(), buff->getName());
    }

    queryCheckError();

    GLint mapped; 
    DIRECT_CALL_CHK(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_MAPPED, &mapped);
    if (mapped == GL_TRUE) {
        throw std::runtime_error("Buffer is currently mapped, cannot access data.");
    } else {
        GLint size = 0; 
        DIRECT_CALL_CHK(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_SIZE, &size);

        queryCheckError();

        if (!size) {
            throw std::runtime_error("Buffer empty (GL_BUFFER_SIZE is 0)");
        } else {
            resource->m_Data.resize(size);
            DIRECT_CALL_CHK(glGetBufferSubData)(buff->getTarget(), 0, size, &resource->m_Data[0]);
        }
    }

    //restore state
    if (lastBuffer != buff->getName()) {
        DIRECT_CALL_CHK(glBindBuffer)(buff->getTarget(), lastBuffer);
    }
    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryFramebuffer(gl_t _bufferEnum) {

    GLuint bufferEnum = static_cast<GLuint>(_bufferEnum);

    dglnet::resource::DGLResourceFramebuffer* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceFramebuffer);

    if (!m_NativeReadSurface) {
        throw std::runtime_error("Buffer does not exist");
    }
    state_setters::ReadBuffer readBuffer;
    state_setters::DefaultPBO defPBO;
    state_setters::CurrentFramebuffer currentFramebuffer(0);
    state_setters::PixelStoreAlignment defAlignment;
    
    //select read buffer
    if (m_Version.check(GLContextVersion::ES, 3) || m_Version.check(GLContextVersion::DT)) {
        DIRECT_CALL_CHK(glReadBuffer)(bufferEnum);
    }


    std::vector<GLint> rgbaSizes(m_NativeReadSurface->getRGBASizes(), m_NativeReadSurface->getRGBASizes() + 4);
    std::vector<GLint> deptStencilSizes;

    deptStencilSizes.push_back(m_NativeReadSurface->getDepthSize());
    deptStencilSizes.push_back(m_NativeReadSurface->getStencilSize());

    int width = m_NativeReadSurface->getWidth();
    int height = m_NativeReadSurface->getHeight();

    //we cannot reliably get internalformat for default framebuffer, so it is 0 here.
    DGLPixelTransfer transfer(rgbaSizes, deptStencilSizes, 0);

    resource->m_PixelRectangle = boost::make_shared<dglnet::resource::DGLPixelRectangle>(width, height, defAlignment.getAligned(width * transfer.getPixelSize()),
        transfer.getFormat(), transfer.getType(), 0, 0);
#pragma message ( "GLContext::queryFramebuffer: query MSAA" )

    GLvoid* ptr;
    if ((ptr = resource->m_PixelRectangle->getPtr()) != NULL)
        DIRECT_CALL_CHK(glReadPixels)(0, 0, width, height, transfer.getFormat(), transfer.getType(), ptr);

    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryFBO(gl_t _name) {

    GLuint name = static_cast<GLuint>(_name);

    dglnet::resource::DGLResourceFBO* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceFBO());
 
    state_setters::ReadBuffer readBuffer;
    state_setters::DrawBuffers drawBuffers; //we may touch draw buffer when downsampling MSAA buffers
    state_setters::RenderBuffer renderBuffer;
    state_setters::DefaultPBO defPBO;
    state_setters::CurrentFramebuffer currentFBO(name);
    state_setters::PixelStoreAlignment defAlignment;

    //get maximum number of color attachments
    GLint maxColorAttachments; 
    DIRECT_CALL_CHK(glGetIntegerv)(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

    //fill table with color attachments to look for
    std::vector<GLenum> attachments(maxColorAttachments);
    for (int i = 0; i < maxColorAttachments; i++) {
        attachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    //additionally we will check some non-color attachments
       
    //it is crucial, that depth_stencil is checked first (we will break is succeed)
    attachments.push_back(GL_DEPTH_STENCIL_ATTACHMENT);

    //if depth_stencil fails, we will also check these:
    attachments.push_back(GL_DEPTH_ATTACHMENT);
    attachments.push_back(GL_STENCIL_ATTACHMENT);

    queryCheckError();

    for (size_t i = 0; i < attachments.size(); i++) {
            
        //check attached object type
        GLint type;
        DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i],
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            
        if (DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR) {
            //sometimes the query above may fail, for example when asking about
            //depth_stencil attachment, when depth attachment != stencil attachment
            continue;
        }

        if (type != GL_TEXTURE && type != GL_RENDERBUFFER) {
            //no object attached:  skip
            continue;
        }

        //it looks, like there is an attachment. Add it to returned list
        resource->m_Attachments.push_back(dglnet::resource::DGLResourceFBO::FBOAttachment(attachments[i]));

        //query attached object name
        GLint attmntName;
        DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i],
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attmntName);
        
        //now look for the attached object and query internal format and dimensions
        GLint width = 0, height = 0, internalFormat = 0, samples = 0;
        GLenum attTarget;
        bool multisampled = false;
        if (type == GL_TEXTURE) {

            if (!DIRECT_CALL_CHK(glIsTexture)(attmntName)) {
                resource->m_Attachments.back().error("Attached texture object does not exist");
                continue;
            }

            GLTextureObj* tex = ensureTexture(attmntName);
            attTarget = tex->getTarget();

            GLenum bindableTarget = glutils::textTargetToBindableTarget(attTarget);

            if (attTarget == GL_TEXTURE_2D_MULTISAMPLE || 
                attTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
                multisampled = true;

            GLint level;
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &level);

            GLint lastTexture = glutils::getBoundTexture(attTarget);
            DIRECT_CALL_CHK(glBindTexture)(bindableTarget, tex->getName());

            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(attTarget, level, GL_TEXTURE_WIDTH, &width);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(attTarget, level, GL_TEXTURE_HEIGHT, &height);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(attTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(attTarget, level, GL_TEXTURE_SAMPLES, &samples);

            DIRECT_CALL_CHK(glBindTexture)(bindableTarget, lastTexture);

        } else if (type == GL_RENDERBUFFER) {
            
            attTarget = GL_RENDERBUFFER;

            if (!DIRECT_CALL_CHK(glIsRenderbuffer)(attmntName)) {
                resource->m_Attachments.back().error("Attached renderbuffer object does not exist");
                continue;
            }
            GLint lastRenderBuffer;
            DIRECT_CALL_CHK(glGetIntegerv)(GL_RENDERBUFFER_BINDING, &lastRenderBuffer);
            DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, attmntName);

            DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
            DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
            DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &internalFormat);
            DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);

            if (samples) {
                multisampled = true;
            }

            DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, lastRenderBuffer);
        } else {
            assert(0);
        }

        //there should be no errors. Otherwise something nasty happened
        queryCheckError();


        std::vector<GLint> rgbaSizes(4, 0);
        std::vector<GLint> deptStencilSizes(2, 0);

        if (attachments[i] >= GL_COLOR_ATTACHMENT0 &&
            attachments[i] < GL_COLOR_ATTACHMENT0 + (GLenum)maxColorAttachments) {
                
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &rgbaSizes[0]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &rgbaSizes[1]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &rgbaSizes[2]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &rgbaSizes[3]);

        } else if (attachments[i] == GL_DEPTH_ATTACHMENT) {

            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &deptStencilSizes[0]);

        } else if (attachments[i] == GL_STENCIL_ATTACHMENT) {

            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &deptStencilSizes[1]);

        } else if (attachments[i] == GL_DEPTH_STENCIL_ATTACHMENT) {

            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &deptStencilSizes[0]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_FRAMEBUFFER, attachments[i], 
                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &deptStencilSizes[1]);

        }
          
        //there should be no errors. Otherwise something nasty happened
        queryCheckError();

        DGLPixelTransfer transfer(rgbaSizes, deptStencilSizes, internalFormat);

        resource->m_Attachments.back().m_PixelRectangle = boost::make_shared<dglnet::resource::DGLPixelRectangle>(width,
            height, defAlignment.getAligned(width * transfer.getPixelSize()), 
            transfer.getFormat(), transfer.getType(), internalFormat, samples);

        boost::shared_ptr<glutils::MSAADownSampler> downSampler;
        if (multisampled) {
            downSampler = boost::make_shared<glutils::MSAADownSampler>(attTarget, attachments[i], name, internalFormat, &transfer, width, height);
            DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, downSampler->getDownsampledFBO());
        }

        if (attachments[i] != GL_DEPTH_ATTACHMENT && attachments[i] != GL_STENCIL_ATTACHMENT && attachments[i] != GL_DEPTH_STENCIL_ATTACHMENT) {
            //select color attachment 
            DIRECT_CALL_CHK(glReadBuffer)(attachments[i]); 
        }

        GLvoid* ptr = resource->m_Attachments.back().m_PixelRectangle->getPtr();
        if (ptr) {
            DIRECT_CALL_CHK(glReadPixels)(0, 0, width, height, transfer.getFormat(), transfer.getType(), ptr);
        }

        //there should be no errors. Otherwise something nasty happened
        queryCheckError();

        if (attachments[i] == GL_DEPTH_STENCIL_ATTACHMENT) {
            //we have succesfully read GL_DEPTH_STENCIL_ATTACHMENT attachment. 
            //WA for buggy drivers: do not try to read DEPTH and STENCIL attachments if DEPTH_STENCIL is used
            break;
        }
    }

    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryShader(gl_t _name) {

    GLuint name = static_cast<GLuint>(_name);

    dglnet::resource::DGLResourceShader* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceShader);

    GLShaderObj* shader = findShader(name);
    if (!shader) {
        throw std::runtime_error("Shader does not exist");
    }    
    
    resource->m_CompileStatus = std::pair<std::string, gl_t>(shader->queryCompilationInfoLog(), shader->queryCompilationStatus());
    
    resource->m_Source = shader->querySources();

    resource->m_ShaderObjDeleted = shader->isDeleted();

    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryProgram(gl_t _name) {

    GLuint name = static_cast<GLuint>(_name);

    dglnet::resource::DGLResourceProgram* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceProgram);

    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name);
    if (i == m_Programs.end()) {
        throw std::runtime_error("Shader Program does not exist");
    }
    GLProgramObj* program = &i->second;

    std::set<GLShaderObj*> attachedShaders = program->getAttachedShaders();
    for (std::set<GLShaderObj*>::iterator i = attachedShaders.begin(); i != attachedShaders.end(); i++) {
        resource->m_AttachedShaders.push_back(std::pair<gl_t, gl_t>((*i)->getName(), (*i)->getTarget()));
    }

    GLint linkStatus, infoLogLength; 
    DIRECT_CALL_CHK(glGetProgramiv)(name, GL_LINK_STATUS, &linkStatus);
    DIRECT_CALL_CHK(glGetProgramiv)(name, GL_INFO_LOG_LENGTH, &infoLogLength);

    queryCheckError();

    std::string infoLog; 
    if (infoLogLength) {
        infoLog.resize(infoLogLength);
        GLint realInfoLogLength;
        DIRECT_CALL_CHK(glGetProgramInfoLog)(program->getName(), static_cast<GLsizei>(infoLog.size()), &realInfoLogLength, &infoLog[0]);
        if (realInfoLogLength < infoLogLength) {
            //highly unlikely, only on buggy drivers
            infoLog.resize(realInfoLogLength);
        }
    }       

    resource->mLinkStatus = std::pair<std::string, gl_t>(infoLog, linkStatus);

    if (resource->mLinkStatus.second) {

        //if the program is linked, acquire it's uniform values

        GLint activeUniforms, activeUniformsMaxNameLength;
        DIRECT_CALL_CHK(glGetProgramiv)(name, GL_ACTIVE_UNIFORMS, &activeUniforms);
        DIRECT_CALL_CHK(glGetProgramiv)(name, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformsMaxNameLength);
        std::vector<GLchar> nameBuffer(activeUniformsMaxNameLength);
        for (int i =0; i < activeUniforms; i++) {
            dglnet::resource::DGLResourceProgram::Uniform uniform;
            uniform.m_supportedType = false;

            GLsizei length;
            GLint size;
            GLenum type;
            DIRECT_CALL_CHK(glGetActiveUniform)(name, i, activeUniformsMaxNameLength, &length, &size, &type, &nameBuffer[0]);

            nameBuffer[length] = 0;
            uniform.m_name = &nameBuffer[0];
            uniform.m_type = type; 

            uniform.m_location = DIRECT_CALL_CHK(glGetUniformLocation)(name, uniform.m_name.c_str());
            if (uniform.m_location >= 0) {
                uniform.m_supportedType = true;
                GLenum baseType = 0;
                int typeSize = 0;
                switch (type) {
                case GL_FLOAT:
                case GL_FLOAT_VEC2:
                case GL_FLOAT_VEC3:
                case GL_FLOAT_VEC4:
                case GL_FLOAT_MAT2:
                case GL_FLOAT_MAT3:
                case GL_FLOAT_MAT4:
                case GL_FLOAT_MAT2x3:
                case GL_FLOAT_MAT2x4:
                case GL_FLOAT_MAT3x2:
                case GL_FLOAT_MAT3x4:
                case GL_FLOAT_MAT4x2:
                case GL_FLOAT_MAT4x3:
                    baseType = GL_FLOAT;
                    break;
                case GL_DOUBLE:
                case GL_DOUBLE_VEC2:
                case GL_DOUBLE_VEC3:
                case GL_DOUBLE_VEC4:
                case GL_DOUBLE_MAT2:
                case GL_DOUBLE_MAT3:
                case GL_DOUBLE_MAT4:
                case GL_DOUBLE_MAT2x3:
                case GL_DOUBLE_MAT2x4:
                case GL_DOUBLE_MAT3x2:
                case GL_DOUBLE_MAT3x4:
                case GL_DOUBLE_MAT4x2:
                case GL_DOUBLE_MAT4x3:
                    baseType = GL_DOUBLE;
                    break;
                case GL_INT:
                case GL_INT_VEC2:
                case GL_INT_VEC3:
                case GL_INT_VEC4:
                    baseType = GL_INT;
                    break;
                case GL_UNSIGNED_INT:
                case GL_UNSIGNED_INT_VEC2:
                case GL_UNSIGNED_INT_VEC3:
                case GL_UNSIGNED_INT_VEC4:
                    baseType = GL_UNSIGNED_INT;
                    break;
                case GL_BOOL:
                case GL_BOOL_VEC2:
                case GL_BOOL_VEC3:
                case GL_BOOL_VEC4:
                    baseType = GL_BOOL;
                    break;
                default:
                    uniform.m_supportedType = false;
                }

                if (uniform.m_supportedType) {
                    switch (type) {
                    case GL_FLOAT:
                    case GL_DOUBLE:
                    case GL_INT:
                    case GL_UNSIGNED_INT:
                    case GL_BOOL:
                        typeSize = 1;
                        uniform.m_rowSize = 1;
                        break;
                    case GL_FLOAT_VEC2:
                    case GL_DOUBLE_VEC2:
                    case GL_INT_VEC2:
                    case GL_UNSIGNED_INT_VEC2:
                    case GL_BOOL_VEC2:
                        typeSize = 2;
                        uniform.m_rowSize = 2;
                        break;
                    case GL_FLOAT_VEC3:
                    case GL_DOUBLE_VEC3:
                    case GL_INT_VEC3:
                    case GL_UNSIGNED_INT_VEC3:
                    case GL_BOOL_VEC3:
                        typeSize = 3;
                        uniform.m_rowSize = 3;
                        break;
                    case GL_FLOAT_VEC4:
                    case GL_DOUBLE_VEC4:
                    case GL_INT_VEC4:
                    case GL_UNSIGNED_INT_VEC4:
                    case GL_BOOL_VEC4:
                        typeSize = 4;
                        uniform.m_rowSize = 4;
                        break;
                    case GL_FLOAT_MAT2:
                    case GL_DOUBLE_MAT2:
                        typeSize = 4;
                        uniform.m_rowSize = 2;
                        break;
                    case GL_FLOAT_MAT3:
                    case GL_DOUBLE_MAT3:
                        typeSize = 9;
                        uniform.m_rowSize = 3;
                        break;
                    case GL_FLOAT_MAT4:
                    case GL_DOUBLE_MAT4:
                        typeSize = 16;
                        uniform.m_rowSize = 4;
                        break;
                    case GL_FLOAT_MAT2x3:
                    case GL_DOUBLE_MAT2x3:
                        typeSize = 6;
                        uniform.m_rowSize = 3;
                        break;
                    case GL_FLOAT_MAT2x4:
                    case GL_DOUBLE_MAT2x4:
                        typeSize = 8;
                        uniform.m_rowSize = 4;
                        break;
                    case GL_FLOAT_MAT3x2:
                    case GL_DOUBLE_MAT3x2:
                        typeSize = 6;
                        uniform.m_rowSize = 2;
                        break;
                    case GL_FLOAT_MAT3x4:
                    case GL_DOUBLE_MAT3x4:
                        typeSize = 12;
                        uniform.m_rowSize = 4;
                        break;
                    case GL_FLOAT_MAT4x2:
                    case GL_DOUBLE_MAT4x2:
                        typeSize = 8;
                        uniform.m_rowSize = 2;
                        break;
                    case GL_FLOAT_MAT4x3:
                    case GL_DOUBLE_MAT4x3:
                        typeSize = 12;
                        uniform.m_rowSize = 3;
                        break;
                    default:
                        assert(0);
                    }

                    //size is 1 for scalars and > 1 for arrays of uniform scalars (see glGetActiveUniform)

                    //typeSize is size of uniform type in terms of baseType elements

                    uniform.m_value.resize(size * typeSize);

                    if (baseType == GL_FLOAT) {
                        std::vector<GLfloat> value(uniform.m_value.size());
                        for (int i = 0; i < size; i++) {
                            DIRECT_CALL_CHK(glGetUniformfv)(name, uniform.m_location + i, &value[i * typeSize]);
                        }
                        std::copy(value.begin(), value.end(), uniform.m_value.begin());
                    } else if (baseType == GL_DOUBLE) {
                        std::vector<GLdouble> value(uniform.m_value.size());
                        for (int i = 0; i < size; i++) {
                            DIRECT_CALL_CHK(glGetUniformdv)(name, uniform.m_location + i, &value[i * typeSize]);
                        }
                        std::copy(value.begin(), value.end(), uniform.m_value.begin());
                    } else if (baseType == GL_INT) {
                        std::vector<GLint> value(uniform.m_value.size());
                        for (int i = 0; i < size; i++) {
                            DIRECT_CALL_CHK(glGetUniformiv)(name, uniform.m_location + i, &value[i * typeSize]);
                        }
                        std::copy(value.begin(), value.end(), uniform.m_value.begin());
                    } else if (baseType == GL_UNSIGNED_INT || baseType == GL_BOOL) {
                        std::vector<GLuint> value(uniform.m_value.size());
                        for (int i = 0; i < size; i++) {
                            DIRECT_CALL_CHK(glGetUniformuiv)(name, uniform.m_location + i, &value[i * typeSize]);
                        }
                        std::copy(value.begin(), value.end(), uniform.m_value.begin());
                    } else  { assert(0); }
                }
            }
            resource->m_Uniforms.push_back(uniform);
        }

    }
    return ret;
}



boost::shared_ptr<dglnet::DGLResource> GLContext::queryGPU() {

    dglnet::resource::DGLResourceGPU* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceGPU);

    // Forgot about:
    //GL_EXTENSIONS
    
    const char* value;

    if ((value = (const char*)DIRECT_CALL_CHK(glGetString)(GL_RENDERER)) != NULL) {
        resource->m_Renderer = value;
    }
    if ((value = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VENDOR)) != NULL) {
        resource->m_Vendor = value;
    }
    if ((value = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VERSION)) != NULL) {
        resource->m_Version = value;
    }
    if ((value = (const char*)DIRECT_CALL_CHK(glGetString)(GL_SHADING_LANGUAGE_VERSION)) != NULL) {
        resource->m_GLSL = value;
    }

    if ((resource->m_hasNVXGPUMemoryInfo = m_HasNVXMemoryInfo) != false) {
        GLint val;

#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

        DIRECT_CALL_CHK(glGetIntegerv)(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &val);
        resource->m_nvidiaMemory.memInfoDedidactedVidMem = val;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &val);
        resource->m_nvidiaMemory.memInfoTotalAvailMem = val;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &val);
        resource->m_nvidiaMemory.memInfoCurrentAvailVidMem = val;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &val);
        resource->m_nvidiaMemory.memInfoEvictionCount = val;
        DIRECT_CALL_CHK(glGetIntegerv)(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &val);
        resource->m_nvidiaMemory.memInfoEvictedMem = val;
    }

    return ret;
}


dglnet::resource::DGLResourceState::StateItem GLContext::getStateIntegerv(const char* name, GLenum value, size_t length) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    std::vector<GLint> val(length, 0);
    
    DIRECT_CALL_CHK(glGetIntegerv)(value, &val[0]);
    
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(val.size());
        for (size_t i = 0; i < length; i++) {
            ret.m_Values[i] = val[i];
        }
    }
    return ret;
}

dglnet::resource::DGLResourceState::StateItem GLContext::getStateInteger64v(const char* name, GLenum value, size_t length) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    std::vector<GLint64> val(length, 0);

    if (gc->getVersion().check(GLContextVersion::DT, 3, 2) || gc->getVersion().check(GLContextVersion::ES, 3)) {
        DIRECT_CALL_CHK(glGetInteger64v)(value, &val[0]);
    } else {
        std::vector<GLint> valInt(length, 0);
        DIRECT_CALL_CHK(glGetIntegerv)(value, &valInt[0]);
        std::copy(valInt.begin(), valInt.end(), val.begin());
    }    

    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(val.size());
        for (size_t i = 0; i < length; i++) {
            ret.m_Values[i] = val[i];
        }
    }
    return ret;
}

dglnet::resource::DGLResourceState::StateItem GLContext::getStateFloatv(const char* name, GLenum value, size_t length) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    std::vector<GLfloat> val(length, 0);

    DIRECT_CALL_CHK(glGetFloatv)(value, &val[0]);

    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(val.size());
        for (size_t i = 0; i < length; i++) {
            ret.m_Values[i] = val[i];
        }
    }
    return ret;                                                      
}

dglnet::resource::DGLResourceState::StateItem GLContext::getStateDoublev(const char* name, GLenum value, size_t length) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    std::vector<GLdouble> val(length, 0);

    DIRECT_CALL_CHK(glGetDoublev)(value, &val[0]);

    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(val.size());
        for (size_t i = 0; i < length; i++) {
            ret.m_Values[i] = val[i];
        }
    }
    return ret;
}

dglnet::resource::DGLResourceState::StateItem GLContext::getStateBooleanv(const char* name, GLenum value, size_t length) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    std::vector<GLboolean> val(length, 0);

    DIRECT_CALL_CHK(glGetBooleanv)(value, &val[0]);

    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(val.size());
        for (size_t i = 0; i < length; i++) {
            ret.m_Values[i] = val[i];
        }
    }
    return ret;                                         
}

dglnet::resource::DGLResourceState::StateItem GLContext::getStateIsEnabled(const char* name, GLenum value) {
    dglnet::resource::DGLResourceState::StateItem ret;
    ret.m_Name = name;
    GLboolean val =  DIRECT_CALL_CHK(glIsEnabled)(value);
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {
        ret.m_Values.resize(1, val);
    }                                          
    return ret;
}

boost::shared_ptr<dglnet::DGLResource> GLContext::queryState(gl_t) {

    dglnet::resource::DGLResourceState* resource;
    boost::shared_ptr<dglnet::DGLResource> ret (resource = new dglnet::resource::DGLResourceState);

#ifdef WA_ES_QUERY_STATE
    if (!gc->getVersion().check(GLContextVersion::DT))
        return ret; //not really supported on non-DT
#endif

#define STATE_INTEGERV(NAME, LENGTH) resource->m_Items.push_back(getStateIntegerv(#NAME, NAME, LENGTH))
#define STATE_INTEGER64V(NAME, LENGTH) resource->m_Items.push_back(getStateInteger64v(#NAME, NAME, LENGTH))
//TODO: we may handle enums properly here
#define STATE_INTEGERENUMV(NAME, LENGTH) resource->m_Items.push_back(getStateIntegerv(#NAME, NAME, LENGTH))
#define STATE_FLOATV(NAME, LENGTH) resource->m_Items.push_back(getStateFloatv(#NAME, NAME, LENGTH))
#define STATE_DOUBLEV(NAME, LENGTH) resource->m_Items.push_back(getStateDoublev(#NAME, NAME, LENGTH))
#define STATE_BOOLEANV(NAME, LENGTH) resource->m_Items.push_back(getStateBooleanv(#NAME, NAME, LENGTH))
#define STATE_ISENABLED(NAME) resource->m_Items.push_back(getStateIsEnabled(#NAME, NAME))


    STATE_INTEGERV(GL_PATCH_VERTICES, 1);
    STATE_FLOATV(GL_PATCH_DEFAULT_OUTER_LEVEL, 4);
    STATE_FLOATV(GL_PATCH_DEFAULT_INNER_LEVEL, 2);
    STATE_INTEGERV(GL_ELEMENT_ARRAY_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_ARRAY_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_DRAW_INDIRECT_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_VERTEX_ARRAY_BINDING, 1);
    STATE_ISENABLED(GL_PRIMITIVE_RESTART);
    STATE_INTEGERV(GL_PRIMITIVE_RESTART_INDEX, 1);
    STATE_INTEGERV(GL_VIEWPORT, 4);
    if (gc->getVersion().check(GLContextVersion::DT)) {
        STATE_DOUBLEV(GL_DEPTH_RANGE, 2);
    } else  {
        STATE_FLOATV(GL_DEPTH_RANGE, 2);
    }
    STATE_ISENABLED(GL_CLIP_DISTANCE0);
    STATE_ISENABLED(GL_CLIP_DISTANCE1);
    STATE_ISENABLED(GL_CLIP_DISTANCE2);
    STATE_ISENABLED(GL_CLIP_DISTANCE3);
    STATE_ISENABLED(GL_CLIP_DISTANCE4);
    STATE_ISENABLED(GL_CLIP_DISTANCE5);
    STATE_ISENABLED(GL_DEPTH_CLAMP);
    STATE_INTEGERV(GL_TRANSFORM_FEEDBACK_BINDING, 1);
    STATE_INTEGERENUMV(GL_CLAMP_READ_COLOR, 1);
    STATE_INTEGERENUMV(GL_PROVOKING_VERTEX, 1);
    STATE_ISENABLED(GL_RASTERIZER_DISCARD);
    STATE_FLOATV(GL_POINT_SIZE, 1);
    STATE_FLOATV(GL_POINT_FADE_THRESHOLD_SIZE, 1);
    STATE_INTEGERENUMV(GL_POINT_SPRITE_COORD_ORIGIN, 1);
    STATE_FLOATV(GL_LINE_WIDTH, 1);
    STATE_ISENABLED(GL_LINE_SMOOTH);
    STATE_ISENABLED(GL_CULL_FACE);
    STATE_INTEGERENUMV(GL_CULL_FACE_MODE, 1);
    STATE_INTEGERENUMV(GL_FRONT_FACE, 1);
    STATE_ISENABLED(GL_POLYGON_SMOOTH);
    STATE_INTEGERENUMV(GL_POLYGON_MODE, 2);
    STATE_FLOATV(GL_POLYGON_OFFSET_FACTOR, 1);
    STATE_FLOATV(GL_POLYGON_OFFSET_UNITS, 1);
    STATE_ISENABLED(GL_POLYGON_OFFSET_POINT);
    STATE_ISENABLED(GL_POLYGON_OFFSET_LINE);
    STATE_ISENABLED(GL_POLYGON_OFFSET_FILL);
    STATE_ISENABLED(GL_MULTISAMPLE);
    STATE_ISENABLED(GL_SAMPLE_ALPHA_TO_COVERAGE);
    STATE_ISENABLED(GL_SAMPLE_ALPHA_TO_ONE);
    STATE_ISENABLED(GL_SAMPLE_COVERAGE);
    STATE_FLOATV(GL_SAMPLE_COVERAGE_VALUE, 1);
    STATE_BOOLEANV(GL_SAMPLE_COVERAGE_INVERT, 1);
    STATE_ISENABLED(GL_SAMPLE_SHADING);
    STATE_FLOATV(GL_MIN_SAMPLE_SHADING_VALUE, 1);
    STATE_ISENABLED(GL_SAMPLE_MASK);
    STATE_INTEGERV(GL_TEXTURE_BINDING_1D, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_2D, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_3D, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_1D_ARRAY, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_2D_ARRAY, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_CUBE_MAP_ARRAY, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_RECTANGLE, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_BUFFER, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_CUBE_MAP, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_2D_MULTISAMPLE, 1);
    STATE_INTEGERV(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, 1);
    STATE_INTEGERV(GL_SAMPLER_BINDING, 1);
    STATE_INTEGERV(GL_ACTIVE_TEXTURE, 1);
    STATE_ISENABLED(GL_SCISSOR_TEST);
    STATE_FLOATV(GL_SCISSOR_BOX, 4);
    STATE_ISENABLED(GL_STENCIL_TEST);
    STATE_INTEGERENUMV(GL_STENCIL_FUNC, 1);
    STATE_INTEGERV(GL_STENCIL_VALUE_MASK, 1);
    STATE_INTEGERV(GL_STENCIL_REF, 1);
    STATE_INTEGERENUMV(GL_STENCIL_FAIL, 1);
    STATE_INTEGERENUMV(GL_STENCIL_PASS_DEPTH_FAIL, 1);
    STATE_INTEGERENUMV(GL_STENCIL_PASS_DEPTH_PASS, 1);
    STATE_INTEGERENUMV(GL_STENCIL_BACK_FUNC, 1);
    STATE_INTEGERV(GL_STENCIL_BACK_VALUE_MASK, 1);
    STATE_INTEGERV(GL_STENCIL_BACK_REF, 1);
    STATE_INTEGERENUMV(GL_STENCIL_BACK_FAIL, 1);
    STATE_INTEGERENUMV(GL_STENCIL_BACK_PASS_DEPTH_FAIL, 1);
    STATE_INTEGERENUMV(GL_STENCIL_BACK_PASS_DEPTH_PASS, 1);
    STATE_ISENABLED(GL_DEPTH_TEST);
    STATE_INTEGERENUMV(GL_DEPTH_FUNC, 1);
    STATE_ISENABLED(GL_BLEND);
    STATE_INTEGERV(GL_BLEND_SRC_RGB, 1);
    STATE_INTEGERV(GL_BLEND_SRC_ALPHA, 1);
    STATE_INTEGERV(GL_BLEND_DST_RGB, 1);
    STATE_INTEGERV(GL_BLEND_DST_ALPHA, 1);
    STATE_INTEGERV(GL_BLEND_EQUATION_RGB, 1);
    STATE_INTEGERV(GL_BLEND_EQUATION_ALPHA, 1);
    STATE_FLOATV(GL_BLEND_COLOR, 4);
    STATE_ISENABLED(GL_FRAMEBUFFER_SRGB);
    STATE_ISENABLED(GL_DITHER);
    STATE_ISENABLED(GL_COLOR_LOGIC_OP);
    STATE_INTEGERENUMV(GL_LOGIC_OP_MODE, 1);
    STATE_BOOLEANV(GL_DEPTH_WRITEMASK, 1);
    STATE_INTEGERV(GL_STENCIL_WRITEMASK, 1);
    STATE_INTEGERV(GL_STENCIL_BACK_WRITEMASK, 1);
    STATE_FLOATV(GL_COLOR_CLEAR_VALUE, 4);
    STATE_FLOATV(GL_DEPTH_CLEAR_VALUE, 1);
    STATE_INTEGERV(GL_STENCIL_CLEAR_VALUE, 1);
    STATE_INTEGERV(GL_DRAW_FRAMEBUFFER_BINDING, 1);
    STATE_INTEGERV(GL_READ_FRAMEBUFFER_BINDING, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER0, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER1, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER2, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER3, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER4, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER5, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER6, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER7, 1);
    STATE_INTEGERV(GL_DRAW_BUFFER8, 1);
    STATE_INTEGERENUMV(GL_READ_BUFFER, 1);
    STATE_INTEGERV(GL_RENDERBUFFER_BINDING, 1);
    STATE_BOOLEANV(GL_UNPACK_SWAP_BYTES, 1);
    STATE_BOOLEANV(GL_UNPACK_LSB_FIRST, 1);
    STATE_INTEGERV(GL_UNPACK_IMAGE_HEIGHT, 1);
    STATE_INTEGERV(GL_UNPACK_SKIP_IMAGES, 1);
    STATE_INTEGERV(GL_UNPACK_ROW_LENGTH, 1);
    STATE_INTEGERV(GL_UNPACK_SKIP_ROWS, 1);
    STATE_INTEGERV(GL_UNPACK_SKIP_PIXELS, 1);
    STATE_INTEGERV(GL_UNPACK_ALIGNMENT, 1);
    STATE_INTEGERV(GL_UNPACK_COMPRESSED_BLOCK_WIDTH, 1);
    STATE_INTEGERV(GL_UNPACK_COMPRESSED_BLOCK_HEIGHT, 1);
    STATE_INTEGERV(GL_UNPACK_COMPRESSED_BLOCK_DEPTH, 1);
    STATE_INTEGERV(GL_UNPACK_COMPRESSED_BLOCK_SIZE, 1);
    STATE_INTEGERV(GL_PIXEL_UNPACK_BUFFER_BINDING, 1);
    STATE_BOOLEANV(GL_PACK_SWAP_BYTES, 1);
    STATE_BOOLEANV(GL_PACK_LSB_FIRST, 1);
    STATE_INTEGERV(GL_PACK_IMAGE_HEIGHT, 1);
    STATE_INTEGERV(GL_PACK_SKIP_IMAGES, 1);
    STATE_INTEGERV(GL_PACK_ROW_LENGTH, 1);
    STATE_INTEGERV(GL_PACK_SKIP_ROWS, 1);
    STATE_INTEGERV(GL_PACK_SKIP_PIXELS, 1);
    STATE_INTEGERV(GL_PACK_ALIGNMENT, 1);
    STATE_INTEGERV(GL_PACK_COMPRESSED_BLOCK_WIDTH, 1);
    STATE_INTEGERV(GL_PACK_COMPRESSED_BLOCK_HEIGHT, 1);
    STATE_INTEGERV(GL_PACK_COMPRESSED_BLOCK_DEPTH, 1);
    STATE_INTEGERV(GL_PACK_COMPRESSED_BLOCK_SIZE, 1);
    STATE_INTEGERV(GL_PIXEL_PACK_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_CURRENT_PROGRAM, 1);
    STATE_INTEGERV(GL_PROGRAM_PIPELINE_BINDING, 1);
    STATE_INTEGERV(GL_UNIFORM_BUFFER_BINDING, 1);
    STATE_ISENABLED(GL_PROGRAM_POINT_SIZE);
    STATE_INTEGERV(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 1);
    STATE_BOOLEANV(GL_TRANSFORM_FEEDBACK_PAUSED, 1);
    STATE_BOOLEANV(GL_TRANSFORM_FEEDBACK_ACTIVE, 1);
    STATE_INTEGERV(GL_ATOMIC_COUNTER_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_SHADER_STORAGE_BUFFER_BINDING, 1);
    STATE_INTEGERENUMV(GL_LINE_SMOOTH_HINT, 1);
    STATE_INTEGERENUMV(GL_POLYGON_SMOOTH_HINT, 1);
    STATE_INTEGERENUMV(GL_TEXTURE_COMPRESSION_HINT, 1);
    STATE_INTEGERENUMV(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, 1);
    STATE_INTEGERV(GL_DISPATCH_INDIRECT_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_MAX_CLIP_DISTANCES, 1);
    STATE_INTEGERV(GL_SUBPIXEL_BITS, 1);
    STATE_INTEGER64V(GL_MAX_ELEMENT_INDEX, 1);
    STATE_INTEGERENUMV(GL_IMPLEMENTATION_COLOR_READ_FORMAT, 1);
    STATE_INTEGERENUMV(GL_IMPLEMENTATION_COLOR_READ_TYPE, 1);
    STATE_INTEGERV(GL_MAX_3D_TEXTURE_SIZE, 1);
    STATE_INTEGERV(GL_MAX_TEXTURE_SIZE, 1);
    STATE_INTEGERV(GL_MAX_ARRAY_TEXTURE_LAYERS, 1);
    STATE_FLOATV(GL_MAX_TEXTURE_LOD_BIAS, 1);
    STATE_INTEGERV(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 1);
    STATE_INTEGERV(GL_MAX_RENDERBUFFER_SIZE, 1);
    STATE_INTEGERV(GL_MAX_VIEWPORTS, 1);
    STATE_INTEGERV(GL_VIEWPORT_SUBPIXEL_BITS, 1);
    STATE_FLOATV(GL_VIEWPORT_BOUNDS_RANGE, 2);
    STATE_INTEGERENUMV(GL_LAYER_PROVOKING_VERTEX, 1);
    STATE_INTEGERENUMV(GL_VIEWPORT_INDEX_PROVOKING_VERTEX, 1);
    STATE_FLOATV(GL_POINT_SIZE_RANGE, 2);
    STATE_FLOATV(GL_POINT_SIZE_GRANULARITY, 1);
    STATE_FLOATV(GL_ALIASED_LINE_WIDTH_RANGE, 2);
    STATE_FLOATV(GL_SMOOTH_LINE_WIDTH_RANGE, 2);
    STATE_FLOATV(GL_SMOOTH_LINE_WIDTH_GRANULARITY, 1);
    STATE_INTEGERV(GL_MAX_ELEMENTS_INDICES, 1);
    STATE_INTEGERV(GL_MAX_ELEMENTS_VERTICES, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_ATTRIB_BINDINGS, 1);
    //STATE_INTEGERV(GL_COMPRESSED_TEXTURE_FORMATS);
    STATE_INTEGERV(GL_NUM_COMPRESSED_TEXTURE_FORMATS, 1);
    STATE_INTEGERV(GL_MAX_TEXTURE_BUFFER_SIZE, 1);
    STATE_INTEGERV(GL_MAX_RECTANGLE_TEXTURE_SIZE, 1);
    STATE_INTEGERV(GL_PROGRAM_BINARY_FORMATS, 1);
    STATE_INTEGERV(GL_NUM_PROGRAM_BINARY_FORMATS, 1);
    STATE_INTEGERV(GL_SHADER_BINARY_FORMATS, 1);
    STATE_INTEGERV(GL_NUM_SHADER_BINARY_FORMATS, 1);
    STATE_BOOLEANV(GL_SHADER_COMPILER, 1);
    STATE_INTEGERV(GL_MIN_MAP_BUFFER_ALIGNMENT, 1);
    STATE_INTEGERV(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, 1);
    STATE_INTEGERV(GL_MAJOR_VERSION, 1);
    STATE_INTEGERV(GL_MINOR_VERSION, 1);
    STATE_INTEGERV(GL_CONTEXT_FLAGS, 1);
    STATE_INTEGERV(GL_NUM_EXTENSIONS, 1);
    STATE_INTEGERV(GL_NUM_SHADING_LANGUAGE_VERSIONS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_ATTRIBS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_UNIFORM_VECTORS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_TESS_GEN_LEVEL, 1);
    STATE_INTEGERV(GL_MAX_PATCH_VERTICES, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_PATCH_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_INPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_OUTPUT_VERTICES, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_SHADER_INVOCATIONS, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_STREAMS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_UNIFORM_VECTORS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_INPUT_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET, 1);
    STATE_INTEGERV(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, 1);
    //STATE_INTEGERV(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_MIN_PROGRAM_TEXEL_OFFSET, 1);
    STATE_INTEGERV(GL_MAX_PROGRAM_TEXEL_OFFSET, 1);
    STATE_INTEGERV(GL_MAX_UNIFORM_BUFFER_BINDINGS, 1);
    STATE_INTEGERV(GL_MAX_UNIFORM_BLOCK_SIZE, 1);
    STATE_INTEGERV(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_UNIFORM_BLOCKS, 1);
    STATE_INTEGERV(GL_MAX_VARYING_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_VARYING_VECTORS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_SUBROUTINES, 1);
    STATE_INTEGERV(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, 1);
    STATE_INTEGERV(GL_MAX_UNIFORM_LOCATIONS, 1);
    STATE_INTEGERV(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, 1);
    STATE_INTEGERV(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_ATOMIC_COUNTERS, 1);
    STATE_INTEGERV(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, 1);
    STATE_INTEGER64V(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, 1);
    STATE_INTEGERV(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, 1);
    STATE_INTEGERV(GL_MAX_IMAGE_UNITS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, 1);
    STATE_INTEGERV(GL_MAX_IMAGE_SAMPLES, 1);
    STATE_INTEGERV(GL_MAX_VERTEX_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_IMAGE_UNIFORMS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, 1);
    STATE_INTEGERV(GL_DEBUG_LOGGED_MESSAGES, 1);
    STATE_INTEGERV(GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH, 1);
    STATE_ISENABLED(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    STATE_INTEGERV(GL_DEBUG_GROUP_STACK_DEPTH, 1);
    STATE_ISENABLED(GL_DEBUG_OUTPUT);
    STATE_INTEGERV(GL_MAX_DEBUG_MESSAGE_LENGTH, 1);
    STATE_INTEGERV(GL_MAX_DEBUG_LOGGED_MESSAGES, 1);
    STATE_INTEGERV(GL_MAX_DEBUG_GROUP_STACK_DEPTH, 1);
    STATE_INTEGERV(GL_MAX_LABEL_LENGTH, 1);
    STATE_INTEGERV(GL_MAX_SAMPLE_MASK_WORDS, 1);
    STATE_INTEGERV(GL_MAX_SAMPLES, 1);
    STATE_INTEGERV(GL_MAX_COLOR_TEXTURE_SAMPLES, 1);
    STATE_INTEGERV(GL_MAX_DEPTH_TEXTURE_SAMPLES, 1);
    STATE_INTEGERV(GL_MAX_INTEGER_SAMPLES, 1);
    STATE_FLOATV(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET, 1);
    STATE_FLOATV(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET, 1);
    STATE_INTEGERV(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS, 1);
    STATE_INTEGERV(GL_MAX_DRAW_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, 1);
    STATE_INTEGERV(GL_MAX_COLOR_ATTACHMENTS, 1);
    STATE_INTEGERV(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, 1);
    STATE_INTEGERV(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, 1);
    STATE_INTEGERV(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, 1);
    STATE_BOOLEANV(GL_DOUBLEBUFFER, 1);
    STATE_BOOLEANV(GL_STEREO, 1);
    STATE_INTEGERV(GL_SAMPLE_BUFFERS, 1);
    STATE_INTEGERV(GL_SAMPLES, 1);
    STATE_INTEGERV(GL_COPY_READ_BUFFER_BINDING, 1);
    STATE_INTEGERV(GL_COPY_WRITE_BUFFER_BINDING, 1);
    return ret;
}

GLProgramObj* GLContext::ensureProgram(GLuint name) {
    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name);
    if (i == m_Programs.end()) {
        i = m_Programs.insert(std::pair<GLuint, GLProgramObj>(name, GLProgramObj(name))).first;
    }
    return &(*i).second;
}

GLProgramObj* GLContext::findProgram(GLuint name) {
    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name);
    if (i == m_Programs.end()) {
        return NULL;
    }
    return &(*i).second;
}

void GLContext::deleteProgram(GLuint name) {
    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name); 
    if (i !=  m_Programs.end()) {
        m_Programs.erase(i);
    }
}

GLShaderObj* GLContext::ensureShader(GLuint name, bool fromArbAPI) {
    std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.find(name);
    if (i == m_Shaders.end()) {
        i = m_Shaders.insert(std::pair<GLuint, GLShaderObj>(name, GLShaderObj(name, fromArbAPI))).first;
    }
    return &(*i).second;
}

GLShaderObj* GLContext::findShader(GLuint name) {
    std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.find(name);
    if (i == m_Shaders.end()) {
        return NULL;
    }
    return &(*i).second;
}


opaque_id_t GLContext::getId() {
    return m_Id;
}

void APIENTRY debugOutputCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum /*severity*/, GLsizei length, const GLchar* message, GLvoid* userParam) {
    reinterpret_cast<GLContext*>(userParam)->setDebugOutput(std::string(message, length));
}


void GLContext::firstUse() {
    GLint maxExtensions; 
    
    std::vector<std::string> exts;
    
    bool debugOutputSupported = false;
    if (gc->getVersion().check(GLContextVersion::DT, 3) || gc->getVersion().check(GLContextVersion::ES, 3)) {
        DIRECT_CALL_CHK(glGetIntegerv)(GL_NUM_EXTENSIONS, &maxExtensions);
        exts.resize(maxExtensions);
        for (int i = 0; i < maxExtensions; i++) {
            exts[i] = (char*)DIRECT_CALL_CHK(glGetStringi)(GL_EXTENSIONS, i);
        }
    } else {
        std::string allExts = (char*)DIRECT_CALL_CHK(glGetString)(GL_EXTENSIONS);
        std::istringstream allExtsStream(allExts);
        std::copy(std::istream_iterator<std::string>(allExtsStream),
            std::istream_iterator<std::string>(),
            std::back_inserter<std::vector<std::string> >(exts));
    }

    for (size_t i = 0; i < exts.size(); i++) {
        if (strcmp("GL_ARB_debug_output", exts[i].c_str()) == 0)
            debugOutputSupported = true;
        if (strcmp("GL_NVX_gpu_memory_info", exts[i].c_str()) == 0)
            m_HasNVXMemoryInfo = true;
    }   
    
    if (debugOutputSupported) {
        DIRECT_CALL_CHK(glEnable)(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        DIRECT_CALL_CHK(glDebugMessageCallbackARB)(debugOutputCallback, this);
    }
}

} //namespace dglState
