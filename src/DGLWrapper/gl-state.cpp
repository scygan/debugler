#include "gl-state.h"

#include <boost/make_shared.hpp>
#include <string>

#include "pointers.h"
#include "api-loader.h"

namespace dglstate {

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

    class DefaultReadBuffer {
    public:
        DefaultReadBuffer(GLuint name = 0) {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_FRAMEBUFFER_BINDING, &m_FBO);
            if (m_FBO) {
                DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, name);
            }
        }
        ~DefaultReadBuffer() {
            if (m_FBO) {
               DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, m_FBO);
            }
        }
    private:
        GLint m_FBO;
    };

    class PixelStoreAlignment {
        static const int StateSize = 8;
    public:
        PixelStoreAlignment() {
            //dump and set pixel store state
            for (int i = 0; i < StateSize; i++) {
                DIRECT_CALL_CHK(glGetIntegerv)(m_Targets[i], &m_SavedState[i]);
                DIRECT_CALL_CHK(glPixelStorei)(m_Targets[i], m_State[i]);
            }
        }
        ~PixelStoreAlignment() {
            for (int i = 0; i < StateSize; i++) {
                DIRECT_CALL_CHK(glPixelStorei)(m_Targets[i], m_SavedState[i]);
            }
        }
    private:
        GLint m_PBO;
        static const GLenum m_Targets[StateSize]; 
        static GLint m_SavedState[StateSize];
        static const GLint m_State[StateSize];
    };
    const GLenum PixelStoreAlignment::m_Targets[StateSize] = {GL_PACK_SWAP_BYTES, GL_PACK_LSB_FIRST, GL_PACK_ROW_LENGTH, GL_PACK_IMAGE_HEIGHT, GL_PACK_SKIP_ROWS,
        GL_PACK_SKIP_PIXELS, GL_PACK_SKIP_IMAGES, GL_PACK_ALIGNMENT};
    const GLint PixelStoreAlignment::m_State[StateSize] =  {GL_FALSE, GL_FALSE, 0, 0, 0, 0, 0, 1 };
    GLint PixelStoreAlignment::m_SavedState[StateSize];
};



GLObj::GLObj():m_Name(0) {}

GLObj::GLObj(GLuint name):m_Name(name) {}

GLuint GLObj::getName() { return m_Name; }


GLTextureObj::GLTextureObj(GLuint name):GLObj(name),m_Target(0) {}

void GLTextureObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLTextureObj::getTarget() {
    return m_Target;
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

void GLProgramObj::use(bool) {
    m_InUse = true;
}

bool GLProgramObj::mayDelete() {
    return !m_InUse && m_Deleted;
}

void GLProgramObj::markDeleted() {
    m_Deleted = true;
}

GLFBObj::GLFBObj(GLuint name):GLObj(name),m_Target(0) {}

void GLFBObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLFBObj::getTarget() {
    return m_Target;
}

GLContext::GLContext(uint32_t id):m_Id(id), m_InUse(false), m_Deleted(false), m_NPISurface(NULL) {}

dglnet::ContextReport GLContext::describe() {
    dglnet::ContextReport ret(m_Id);
    for (std::map<GLuint, GLTextureObj>::iterator i = m_Textures.begin(); 
        i != m_Textures.end(); i++) {
            ret.m_TextureSpace.insert(i->second.getName());
    }
    for (std::map<GLuint, GLBufferObj>::iterator i = m_Buffers.begin(); 
        i != m_Buffers.end(); i++) {
            ret.m_BufferSpace.insert(i->second.getName());
    }
    for (std::map<GLuint, GLProgramObj>::iterator i = m_Programs.begin(); 
        i != m_Programs.end(); i++) {
            ret.m_ProgramSpace.insert(i->second.getName());
    }
    for (std::map<GLuint, GLFBObj>::iterator i = m_FBOs.begin(); 
        i != m_FBOs.end(); i++) {
            ret.m_FBOSpace.insert(i->second.getName());
    }
    if (m_NPISurface) {
        if (m_NPISurface->isStereo()) {
            if (m_NPISurface->isDoubleBuffered()) {
                ret.m_FramebufferSpace.insert(GL_BACK_RIGHT);
            }
            ret.m_FramebufferSpace.insert(GL_FRONT_RIGHT);
        }
        if (m_NPISurface->isDoubleBuffered()) {
            ret.m_FramebufferSpace.insert(GL_BACK);
        }
        ret.m_FramebufferSpace.insert(GL_FRONT);
    }
    return ret;
}

void GLContext::use(bool inUse) {
    m_InUse = inUse;
}

bool GLContext::lazyDelete() {
    m_Deleted = true;
    return !m_InUse;
}

bool GLContext::isDeleted() {
    return m_Deleted;
}

NPISurface* GLContext::getNpiSurface() {
    return m_NPISurface;
}

void GLContext::setNpiSurface(NPISurface* surface) {
    m_NPISurface = surface;
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

GLenum GLContext::getError() {
    GLenum ret;
    if (m_PokedErrorQueue.size()) {
        ret = m_PokedErrorQueue.front();
        m_PokedErrorQueue.pop();
    } else {
        ret = DIRECT_CALL_CHK(glGetError)();
    }
    return ret;
}

GLenum GLContext::peekError() {
    GLenum ret = DIRECT_CALL_CHK(glGetError)();
    if (ret != GL_NO_ERROR) {
        GLenum error = ret;
        do {
            m_PokedErrorQueue.push(error);
            error = DIRECT_CALL_CHK(glGetError)();
        } while (error != GL_NO_ERROR);
    }
    return ret;
}

void GLContext::startQuery() {
    peekError();
}

bool GLContext::endQuery(std::string& message) {
    bool ret = true;
    GLenum error;
    if  ((error = DIRECT_CALL_CHK(glGetError)()) != GL_NO_ERROR ) {
        message = std::string("Query failed: got OpenGL error (") + GetGLEnumName(error) + ")";
        ret = false;
    }
    while (DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR);
    return ret;
}

void GLContext::queryTexture(GLuint name, dglnet::TextureMessage& ret) {

    startQuery();

    ret.m_TextureName = name;
    
    try {
        //check if GL knows about a texture
        if (DIRECT_CALL_CHK(glIsTexture)(name) != GL_TRUE) {
            ret.error("Texture does not exist");
            return;
        }

        //check if we know about a texture target
        GLTextureObj* tex = ensureTexture(name);
        if (tex->getTarget() == 0) {
            ret.error("Texture target is unknown");
            return;
        } else if (tex->getTarget() != GL_TEXTURE_2D) {
            ret.error("Texture target is unsupported");
            return;
        }

        //disconnect PBO if it exists
        state_setters::DefaultPBO defPBO;
        state_setters::PixelStoreAlignment defAlignment;

        //rebind texture, so we can access it
        GLint lastTexture = 0;
        switch (tex->getTarget()) {
        case GL_TEXTURE_2D:
            DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_2D, &lastTexture);
            break;
        default:
            assert(0);
        }
        if (lastTexture != tex->getName()) {
            DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), tex->getName());
        }

        {
            int level = 0;
            dglnet::TextureLevel texLevel;
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &texLevel.m_Width);
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_HEIGHT, &texLevel.m_Height);
            GLint alpha;
            GLenum format;
            DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_ALPHA_SIZE, &alpha);
            if (alpha) {
                texLevel.m_Channels = 4;
                format = GL_BGRA;
            } else {
                texLevel.m_Channels = 3;
                format = GL_RGB;
            }

            texLevel.m_Pixels.resize(texLevel.m_Width * texLevel.m_Height * texLevel.m_Channels);
            DIRECT_CALL_CHK(glGetTexImage)(tex->getTarget(), level, format, GL_UNSIGNED_BYTE, &texLevel.m_Pixels[0]);
            ret.m_Levels.push_back(texLevel);
        }

        //restore state
        if (lastTexture != tex->getName()) {
            DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), lastTexture);
        }
    } catch (const std::runtime_error& err) {
        ret.error(err.what());
    }
    std::string message;
    if (!endQuery(message)) {
        ret.error(message);
    }
}

void GLContext::queryBuffer(GLuint name, dglnet::BufferMessage& ret) {

    startQuery();

    ret.m_BufferName = name;
    try {
        //check if GL knows about a texture
        if (DIRECT_CALL_CHK(glIsBuffer)(name) != GL_TRUE) {
            ret.error("Buffer does not exist");
            return;
        }

        //check if we know about a texture target
        GLBufferObj* buff = ensureBuffer(name);
        if (buff->getTarget() == 0) {
            ret.error("Buffer target is unknown");
            return;
        }

        //rebind buffer, so we can access it
        GLint lastBuffer = 0;
        switch (buff->getTarget()) {
            case GL_ARRAY_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_ATOMIC_COUNTER_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_ATOMIC_COUNTER_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_COPY_READ_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_COPY_READ_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_COPY_WRITE_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_COPY_WRITE_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_DRAW_INDIRECT_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_DRAW_INDIRECT_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_DISPATCH_INDIRECT_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_ELEMENT_ARRAY_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_PIXEL_PACK_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_PIXEL_UNPACK_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)( GL_PIXEL_UNPACK_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_SHADER_STORAGE_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_SHADER_STORAGE_BUFFER_BINDING, &lastBuffer);
                break;
            /*case GL_TEXTURE_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_NO_IDEA_WHAT_SHOULD_BE_HERE, &lastBuffer);
                break;*/
            case GL_TRANSFORM_FEEDBACK_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &lastBuffer);
                break;
            case GL_UNIFORM_BUFFER:
                DIRECT_CALL_CHK(glGetIntegerv)(GL_UNIFORM_BUFFER_BINDING, &lastBuffer);
                break;       
        default:
            ret.error("Buffer target is not supported");
            return;
        }
        if (lastBuffer != buff->getName()) {
            DIRECT_CALL_CHK(glBindBuffer)(buff->getTarget(), buff->getName());
        }

        GLint mapped; 
        DIRECT_CALL_CHK(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_MAPPED, &mapped);
        if (mapped == GL_TRUE) {
            ret.error("Buffer is currently mapped, cannot access data.");
        } else {
            GLint size = 0; 
            DIRECT_CALL_CHK(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_SIZE, &size);
            if (!size) {
                ret.error("Buffer empty (GL_BUFFER_SIZE is 0)");
            } else {
                ret.m_Data.resize(size);
                DIRECT_CALL_CHK(glGetBufferSubData)(buff->getTarget(), 0, size, &ret.m_Data[0]);
            }
        }

        //restore state
        if (lastBuffer != buff->getName()) {
            DIRECT_CALL_CHK(glBindBuffer)(buff->getTarget(), lastBuffer);
        }
    } catch (const std::runtime_error& err) {
        ret.error(err.what());
    }
    std::string message;
    if (!endQuery(message)) {
        ret.error(message);
    }
}

void GLContext::queryFramebuffer(GLuint bufferEnum, dglnet::FramebufferMessage& ret) {

    startQuery();

    ret.m_BufferEnum = bufferEnum;

    try {
        if (!m_NPISurface) {
            ret.error("Buffer does not exist");
            return;
        }
        //save state
        GLint currentBuffer; 
        DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &currentBuffer);
        state_setters::DefaultPBO defPBO;
        state_setters::DefaultReadBuffer defReadFBO;
        state_setters::PixelStoreAlignment defAlignment;

        //read the buffer
        DIRECT_CALL_CHK(glReadBuffer)(bufferEnum);

        ret.m_Width = m_NPISurface->getWidth();
        ret.m_Height = m_NPISurface->getHeight();
        GLenum format = GL_RGB;
        ret.m_Channels = 3;
        if (m_NPISurface->isAlpha()) {
            ret.m_Channels = 4;
            format = GL_BGRA;
        }    
        ret.m_Pixels.resize(ret.m_Width * ret.m_Height * ret.m_Channels);
        DIRECT_CALL_CHK(glReadPixels)(0, 0, ret.m_Width, ret.m_Height, format, GL_UNSIGNED_BYTE, &ret.m_Pixels[0]);
        //restore state
        DIRECT_CALL_CHK(glReadBuffer)(currentBuffer);
    } catch (const std::runtime_error& err) {
        ret.error(err.what());
    }
    std::string message;
    if (!endQuery(message)) {
        ret.error(message);
    }
}

void GLContext::queryFBO(GLuint name, dglnet::FBOMessage& ret) {
    startQuery();

    ret.m_Name = name;
    ret.error("unimplemented");
    /*
    try {
 
        //save state
        GLint currentBuffer; 
        DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &currentBuffer);
        state_setters::DefaultPBO defPBO;
        state_setters::DefaultReadBuffer defReadFBO(name);
        state_setters::PixelStoreAlignment defAlignment;

        //read the buffer
        DIRECT_CALL_CHK(glReadBuffer)(GL_FRONT);

        ret.m_Width = m_NPISurface->getWidth();
        ret.m_Height = m_NPISurface->getHeight();
        GLenum format = GL_RGB;
        ret.m_Channels = 3;
        if (m_NPISurface->isAlpha()) {
            ret.m_Channels = 4;
            format = GL_BGRA;
        }    
        ret.m_Pixels.resize(ret.m_Width * ret.m_Height * ret.m_Channels);
        DIRECT_CALL_CHK(glReadPixels)(0, 0, ret.m_Width, ret.m_Height, format, GL_UNSIGNED_BYTE, &ret.m_Pixels[0]);
        //restore state
        DIRECT_CALL_CHK(glReadBuffer)(currentBuffer);
    } catch (const std::runtime_error& err) {
        ret.error(err.what());
    }*/

    std::string message;
    if (!endQuery(message)) {
        ret.error(message);
    }
}

GLProgramObj* GLContext::ensureProgram(GLuint name) {
    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name);
    if (i == m_Programs.end()) {
        i = m_Programs.insert(std::pair<GLuint, GLProgramObj>(name, GLProgramObj(name))).first;
    }
    return &(*i).second;
}

void GLContext::deleteProgram(GLuint name) {
    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name); 
    if (i !=  m_Programs.end()) {
        m_Programs.erase(i);
    }
}

int32_t GLContext::getId() {
    return m_Id;
}

NPISurface::NPISurface(uint32_t id):m_Id(id) {
    HDC hdc = reinterpret_cast<HDC>(id);
    int i = DIRECT_CALL_CHK(wglGetPixelFormat)(hdc);
    PIXELFORMATDESCRIPTOR  pfd;
    DIRECT_CALL_CHK(wglDescribePixelFormat)(hdc, i, 
        sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    m_DoubleBuffered = (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;
    m_Stereo = (pfd.dwFlags & PFD_STEREO) != 0;
    m_Alpha = (pfd.cAlphaBits != 0);

    RECT rc;
    GetClientRect(WindowFromDC(hdc), &rc);
    m_Width = rc.right - rc.left;
    m_Height = rc.bottom - rc.top;
}


uint32_t NPISurface::getId() {
    return m_Id;
}

bool NPISurface::isDoubleBuffered() {
    return m_DoubleBuffered;
}

bool NPISurface::isStereo() {
    return m_Stereo;
}

bool NPISurface::isAlpha() {
    return m_Alpha;
}

int NPISurface::getWidth() {
    return m_Width;
}

int NPISurface::getHeight() {
    return m_Height;
}


} //namespace dglstate