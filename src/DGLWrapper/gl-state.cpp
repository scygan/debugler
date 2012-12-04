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
        DefaultReadBuffer() {
            DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_FRAMEBUFFER_BINDING, &m_FBO);
        }
        ~DefaultReadBuffer() {
            if (m_FBO) {
               DIRECT_CALL_CHK(glBindFramebuffer)(GL_READ_FRAMEBUFFER, m_FBO);
            }
        }
    private:
        GLint m_FBO;
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

void GLProgramObj::attachShader(GLShaderObj* shader) {
    m_AttachedShaders.insert(shader);

}

void GLProgramObj::detachShader(GLShaderObj* shader) {
    m_AttachedShaders.erase(shader);
}

std::set<GLShaderObj*>& GLProgramObj::getAttachedShaders() {
    return m_AttachedShaders;
}

GLShaderObj::GLShaderObj(GLuint name):GLObj(name), m_Deleted(false), m_Target(0) {
    m_CompileStatus.second = GL_FALSE;
}

void GLShaderObj::markDeleted() {
    m_Deleted = true;
}

void GLShaderObj::setSources(const std::vector<std::string>& sources) {
    m_Sources = sources;
}

void GLShaderObj::setCompilationStatus(const std::string& compileLog, GLint compileStatus) {
    m_CompileStatus.first = compileLog; 
    m_CompileStatus.second = compileStatus;
}

const std::vector<std::string>& GLShaderObj::getSources() const {
    return m_Sources;   
}

GLenum GLShaderObj::getTarget() const {
    return m_Target;   
}

const std::pair<std::string, GLint>& GLShaderObj::getCompileStatus() const {
    return m_CompileStatus;
}

void GLShaderObj::setTarget( GLenum target ) {
    m_Target = target;
}


GLFBObj::GLFBObj(GLuint name):GLObj(name),m_Target(0) {}

void GLFBObj::setTarget(GLenum target) {
    if (!m_Target)
        m_Target = target;
}

GLenum GLFBObj::getTarget() {
    return m_Target;
}

GLContext::GLContext(uint32_t id):m_Id(id), m_InUse(false), m_Deleted(false), m_NPISurface(NULL), m_EverUsed(false), m_HasDebugOutput(false), m_InImmediateMode(false)  {}

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
    for (std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.begin(); 
        i != m_Shaders.end(); i++) {
            ret.m_ShaderSpace.insert(dglnet::ContextObjectNameTarget(i->second.getName(), i->second.getTarget()));
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
    if (!m_EverUsed) {
        firstUse();
        m_EverUsed = true;
    }
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
        if (!m_InImmediateMode) {
            ret = DIRECT_CALL_CHK(glGetError)();
        } else {
            ret = GL_NO_ERROR;
        }
    }
    return ret;
}

GLenum GLContext::peekError() {

    if (m_InImmediateMode) return NO_ERROR; //we cannot get erros after glBegin()

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
    if  ((error = DIRECT_CALL_CHK(glGetError)()) != GL_NO_ERROR ) {
        message = std::string("Query failed: got OpenGL error (") + GetGLEnumName(error) + ")";
        ret = false;
    }
    while (DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR);
    
    //alway invalidate debug output from query functions
    m_HasDebugOutput = false;

    return ret;
}

void GLContext::setImmediateMode(bool immed) {
    m_InImmediateMode = immed;
}

boost::shared_ptr<DGLResource> GLContext::queryTexture(GLuint name) {

    DGLResourceTexture* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceTexture());

    //check if GL knows about a texture
    if (DIRECT_CALL_CHK(glIsTexture)(name) != GL_TRUE) {
        throw std::runtime_error("Texture does not exist");
    }

    //check if we know about a texture target
    GLTextureObj* tex = ensureTexture(name);
    if (tex->getTarget() == 0) {
        throw std::runtime_error("Texture target is unknown");
    } else if (tex->getTarget() != GL_TEXTURE_2D) {
        throw std::runtime_error("Texture target is unsupported");
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
        DGLResourceTexture::TextureLevel texLevel;
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

        queryCheckError();

        texLevel.m_Pixels.resize(texLevel.m_Width * texLevel.m_Height * texLevel.m_Channels);
        DIRECT_CALL_CHK(glGetTexImage)(tex->getTarget(), level, format, GL_UNSIGNED_BYTE, &texLevel.m_Pixels[0]);
        resource->m_Levels.push_back(texLevel);
    }

    //restore state
    if (lastTexture != tex->getName()) {
        DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), lastTexture);
    }
    return ret;
}

boost::shared_ptr<DGLResource> GLContext::queryBuffer(GLuint name) {

    DGLResourceBuffer* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceBuffer());

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
        throw std::runtime_error("Buffer target is not supported");
    }
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

boost::shared_ptr<DGLResource> GLContext::queryFramebuffer(GLuint bufferEnum) {
    DGLResourceFramebuffer* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceFramebuffer);

    if (!m_NPISurface) {
        throw std::runtime_error("Buffer does not exist");
    }
    //save state
    GLint currentBuffer; 
    {
        DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &currentBuffer);
        state_setters::DefaultPBO defPBO;
        state_setters::DefaultReadBuffer defReadFBO;
        state_setters::PixelStoreAlignment defAlignment;

        //read the buffer
        DIRECT_CALL_CHK(glReadBuffer)(bufferEnum);

        resource->m_Width = m_NPISurface->getWidth();
        resource->m_Height = m_NPISurface->getHeight();
        GLenum format = GL_RGB;
        resource->m_Channels = 3;
        if (m_NPISurface->isAlpha()) {
            resource->m_Channels = 4;
            format = GL_BGRA;
        }    
        resource->m_Pixels.resize(resource->m_Width * resource->m_Height * resource->m_Channels);
        DIRECT_CALL_CHK(glReadPixels)(0, 0, resource->m_Width, resource->m_Height, format, GL_UNSIGNED_BYTE, &resource->m_Pixels[0]);
    }
        
    //restore state
    DIRECT_CALL_CHK(glReadBuffer)(currentBuffer);

    return ret;
}

boost::shared_ptr<DGLResource> GLContext::queryFBO(GLuint name) {

    DGLResourceFBO* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceFBO());
 
    //save state
    GLint currentBuffer; 
    DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &currentBuffer);
    {
        state_setters::DefaultPBO defPBO;
        state_setters::CurrentFramebuffer currentFBO(name);
        state_setters::PixelStoreAlignment defAlignment;

        GLint maxColorAttachments; 
        DIRECT_CALL_CHK(glGetIntegerv)(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

        for (int i = 0; i < maxColorAttachments; i++) {
            GLint type, name, alphaSize;
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            if (type != GL_TEXTURE && type != GL_RENDERBUFFER)
                continue;

            resource->m_Attachments.push_back(DGLResourceFBO::FBOAttachment(GL_COLOR_ATTACHMENT0 + i));

            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &alphaSize);

            queryCheckError();

            GLint width, height;
            if (type == GL_TEXTURE) {
                if (!DIRECT_CALL_CHK(glIsTexture)(name)) {
                    resource->m_Attachments.back().error("Attached texture object does not exist");
                    continue;
                }

                GLTextureObj* tex = ensureTexture(name);
                if (tex->getTarget() != GL_TEXTURE_2D) {
                    resource->m_Attachments.back().error("Attached texture target is not supported");
                }

                GLint level;
                DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                    GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &level);

                GLint lastTexture;
                DIRECT_CALL_CHK(glGetIntegerv)(GL_TEXTURE_BINDING_2D, &lastTexture);
                DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), tex->getName());

                DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &width);
                DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &height);
                DIRECT_CALL_CHK(glBindTexture)(tex->getTarget(), lastTexture);

            } else if (type == GL_RENDERBUFFER) {
                if (!DIRECT_CALL_CHK(glIsRenderbuffer)(name)) {
                    resource->m_Attachments.back().error("Attached renderbuffer object does not exist");
                    continue;
                }
                GLint lastRenderBuffer;
                DIRECT_CALL_CHK(glGetIntegerv)(GL_RENDERBUFFER_BINDING, &lastRenderBuffer);
                DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, name);
                DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
                DIRECT_CALL_CHK(glGetRenderbufferParameteriv)(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
                DIRECT_CALL_CHK(glBindRenderbuffer)(GL_RENDERBUFFER, lastRenderBuffer);
            }

            queryCheckError();

            GLenum format;
            if (alphaSize) {
                resource->m_Attachments.back().m_Channels = 4;
                format = GL_BGRA;
            } else {
                resource->m_Attachments.back().m_Channels = 3;
                format = GL_RGB;
            }

            resource->m_Attachments.back().m_Width = width;
            resource->m_Attachments.back().m_Height = height;

            resource->m_Attachments.back().m_Pixels.resize(
                resource->m_Attachments.back().m_Width * 
                resource->m_Attachments.back().m_Height * 
                resource->m_Attachments.back().m_Channels
                );

            DIRECT_CALL_CHK(glReadBuffer)(GL_COLOR_ATTACHMENT0 + i);
            DIRECT_CALL_CHK(glReadPixels)(0, 0, resource->m_Attachments.back().m_Width,
                resource->m_Attachments.back().m_Height,
                format, GL_UNSIGNED_BYTE, &resource->m_Attachments.back().m_Pixels[0]);
        }
    }
    //restore state
    DIRECT_CALL_CHK(glReadBuffer)(currentBuffer);

    return ret;
}

boost::shared_ptr<DGLResource> GLContext::queryShader(GLuint name) {

    DGLResourceShader* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceShader);

    std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.find(name);
    if (i == m_Shaders.end()) {
        throw std::runtime_error("Shader does not exist");
    }
    const GLShaderObj* shader = &i->second;
    resource->m_CompileStatus = shader->getCompileStatus();
    resource->m_Sources = shader->getSources();

    return ret;
}

boost::shared_ptr<DGLResource> GLContext::queryProgram(GLuint name) {

    DGLResourceProgram* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceProgram);

    std::map<GLuint, GLProgramObj>::iterator i = m_Programs.find(name);
    if (i == m_Programs.end()) {
        throw std::runtime_error("Shader Program does not exist");
    }
    GLProgramObj* program = &i->second;

    std::set<GLShaderObj*> attachedShaders = program->getAttachedShaders();
    for (std::set<GLShaderObj*>::iterator i = attachedShaders.begin(); i != attachedShaders.end(); i++) {
        resource->m_AttachedShaders.push_back(std::pair<uint32_t, uint32_t>((*i)->getName(), (*i)->getTarget()));
    }

    GLint linkStatus, infoLogLength; 
    DIRECT_CALL_CHK(glGetProgramiv)(name, GL_LINK_STATUS, &linkStatus);
    DIRECT_CALL_CHK(glGetProgramiv)(name, GL_INFO_LOG_LENGTH, &infoLogLength);

    queryCheckError();

    std::string infoLog; 
    if (infoLogLength) {
        infoLog.resize(infoLogLength);
        GLint realInfoLogLength;
        DIRECT_CALL_CHK(glGetProgramInfoLog)(program->getName(), infoLog.size(), &realInfoLogLength, &infoLog[0]);
        if (realInfoLogLength < infoLogLength) {
            //highly unlikely, only on buggy drivers
            infoLog.resize(realInfoLogLength);
        }
    }       

    resource->mLinkStatus = std::pair<std::string, uint32_t>(infoLog, linkStatus);

    return ret;
}

boost::shared_ptr<DGLResource> GLContext::queryGPU(GLuint name) {

    DGLResourceGPU* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceGPU);

    resource->m_Renderer = (const char*)DIRECT_CALL_CHK(glGetString)(GL_RENDERER);
    resource->m_Vendor   = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VENDOR);
    resource->m_Version  = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VERSION);

    return ret;
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

GLShaderObj* GLContext::ensureShader(GLuint name) {
    std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.find(name);
    if (i == m_Shaders.end()) {
        i = m_Shaders.insert(std::pair<GLuint, GLShaderObj>(name, GLShaderObj(name))).first;
    }
    return &(*i).second;
}

void GLContext::markShaderDeleted(GLuint name) {
    std::map<GLuint, GLShaderObj>::iterator i = m_Shaders.find(name); 
    if (i !=  m_Shaders.end()) {
        i->second.markDeleted();
    }
}


int32_t GLContext::getId() {
    return m_Id;
}

void APIENTRY debugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,  const GLchar* message, GLvoid* userParam) {
    reinterpret_cast<GLContext*>(userParam)->setDebugOutput(std::string(message, length));
};


void GLContext::firstUse() {
    GLint maxExtensions; 
    DIRECT_CALL_CHK(glGetIntegerv)(GL_NUM_EXTENSIONS, &maxExtensions);
    
    bool debugOutputSupported = false;

    for (int i = 0; i < maxExtensions; i++) {
        const char* ext = (char*)DIRECT_CALL_CHK(glGetStringi)(GL_EXTENSIONS, i);
        if (strcmp("GL_ARB_debug_output", ext) == 0)
            debugOutputSupported = true;
    }
    if (debugOutputSupported) {
        DIRECT_CALL_CHK(glEnable)(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        DIRECT_CALL_CHK(glDebugMessageCallbackARB)(debugOutputCallback, this);
    }
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