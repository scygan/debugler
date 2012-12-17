#include "gl-state.h"

#include <boost/make_shared.hpp>
#include <string>

#include "pointers.h"
#include "api-loader.h"

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

GLContext::GLContext(uint32_t id):m_Id(id), m_InUse(false), m_Deleted(false), m_NativeSurface(NULL), m_EverUsed(false), m_HasNVXMemoryInfo(false), m_HasDebugOutput(false), m_InImmediateMode(false)  {}

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
    if (m_NativeSurface) {
        if (m_NativeSurface->isStereo()) {
            if (m_NativeSurface->isDoubleBuffered()) {
                ret.m_FramebufferSpace.insert(GL_BACK_RIGHT);
            }
            ret.m_FramebufferSpace.insert(GL_FRONT_RIGHT);
        }
        if (m_NativeSurface->isDoubleBuffered()) {
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

NativeSurface* GLContext::getNativeSurface() {
    return m_NativeSurface;
}

void GLContext::setNativeSurface(NativeSurface* surface) {
    m_NativeSurface = surface;
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

    for (int level = 0; true; level++) {
        DGLPixelRectangle texLevel;
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &texLevel.m_Width);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_HEIGHT, &texLevel.m_Height);
        if (!texLevel.m_Width || !texLevel.m_Height || DIRECT_CALL_CHK(glGetError)() != GL_NO_ERROR)
            break;
        
        GLint rgba[4] = {0, 0, 0, 0};
        GLint stencil = 0, depth = 0;
        
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_RED_SIZE, &rgba[0]);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_GREEN_SIZE, &rgba[1]);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_BLUE_SIZE, &rgba[2]);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_ALPHA_SIZE, &rgba[3]);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_STENCIL_SIZE, &stencil);
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_DEPTH_SIZE, &depth);

        GLenum colorFormats[4] = { GL_RED, GL_RG, GL_RGB, GL_BGRA }; //last element is BGRA intentionally (viewer will handle DX format faster)

        GLenum format = 0;
        texLevel.m_Channels = 0;
        for (int i = 0; i < 4; i++) {
            if (rgba[i]) {
                texLevel.m_Channels = i + 1;
                format = colorFormats[i];
            }
        }
        if (!format) {
            // not a color texture
            if (depth) {
                format = GL_DEPTH_COMPONENT; texLevel.m_Channels = 1;
            }
            if (stencil) {
                format = GL_STENCIL_INDEX; texLevel.m_Channels = 1;
            }
            if (stencil && depth) {
                format = GL_DEPTH_STENCIL; texLevel.m_Channels = 2;
            }
        }

        queryCheckError();

        if (!format) {
            throw std::runtime_error("Checked texture red, green, blue, alpha, stencil and depth size - all are 0. Unrecognized format.");
        }

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

    if (!m_NativeSurface) {
        throw std::runtime_error("Buffer does not exist");
    }
    //save state
    GLint currentBuffer; 
    {
        DIRECT_CALL_CHK(glGetIntegerv)(GL_READ_BUFFER, &currentBuffer);
        state_setters::DefaultPBO defPBO;
        state_setters::CurrentFramebuffer currentFramebuffer(0);
        state_setters::PixelStoreAlignment defAlignment;

        //read the buffer
        DIRECT_CALL_CHK(glReadBuffer)(bufferEnum);


        GLenum colorFormats[4] = { GL_RED, GL_RG, GL_RGB, GL_BGRA }; //last element is BGRA intentionally (viewer will handle DX format faster)

        GLenum format = 0;


        resource->m_PixelRectangle.m_Width = m_NativeSurface->getWidth();
        resource->m_PixelRectangle.m_Height = m_NativeSurface->getHeight();

        resource->m_PixelRectangle.m_Channels = 0;
        for (int i = 0; i < 4; i++) {
            if (m_NativeSurface->getRGBASizes()[i]) {
                resource->m_PixelRectangle.m_Channels = i + 1;
                format = colorFormats[i];
            }
        }
        if (!format) {
            // not a color texture
            if (m_NativeSurface->getDepthSize()) {
                format = GL_DEPTH_COMPONENT; resource->m_PixelRectangle.m_Channels = 1;
            }
            if (m_NativeSurface->getStencilSize()) {
                format = GL_STENCIL_INDEX; resource->m_PixelRectangle.m_Channels = 1;
            }
            if (m_NativeSurface->getStencilSize() && m_NativeSurface->getDepthSize()) {
                format = GL_DEPTH_STENCIL; resource->m_PixelRectangle.m_Channels = 2;
            }
        }       

        resource->m_PixelRectangle.m_Pixels.resize(resource->m_PixelRectangle.m_Width * resource->m_PixelRectangle.m_Height * resource->m_PixelRectangle.m_Channels);
        DIRECT_CALL_CHK(glReadPixels)(0, 0, resource->m_PixelRectangle.m_Width, resource->m_PixelRectangle.m_Height, format, GL_UNSIGNED_BYTE, &resource->m_PixelRectangle.m_Pixels[0]);
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
            GLint type, name;
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            if (type != GL_TEXTURE && type != GL_RENDERBUFFER)
                continue;

            resource->m_Attachments.push_back(DGLResourceFBO::FBOAttachment(GL_COLOR_ATTACHMENT0 + i));

            GLint rgba[4] = {0, 0, 0, 0};
            GLint stencil = 0, depth = 0;

            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &rgba[0]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &rgba[1]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &rgba[2]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &rgba[3]);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);
            DIRECT_CALL_CHK(glGetFramebufferAttachmentParameteriv)(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);

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

            GLenum colorFormats[4] = { GL_RED, GL_RG, GL_RGB, GL_BGRA }; //last element is BGRA intentionally (viewer will handle DX format faster)

            GLenum format = 0;
           
            resource->m_Attachments.back().m_PixelRectangle.m_Channels = 0;
            for (int j = 0; j < 4; j++) {
                if (rgba[j]) {
                    resource->m_Attachments.back().m_PixelRectangle.m_Channels = j + 1;
                    format = colorFormats[j];
                }
            }
            if (!format) {
                // not a color texture
                if (depth) {
                    format = GL_DEPTH_COMPONENT; resource->m_Attachments.back().m_PixelRectangle.m_Channels = 1;
                }
                if (stencil) {
                    format = GL_STENCIL_INDEX; resource->m_Attachments.back().m_PixelRectangle.m_Channels = 1;
                }
                if (stencil && depth) {
                    format = GL_DEPTH_STENCIL; resource->m_Attachments.back().m_PixelRectangle.m_Channels = 2;
                }
            }

            resource->m_Attachments.back().m_PixelRectangle.m_Width = width;
            resource->m_Attachments.back().m_PixelRectangle.m_Height = height;

            resource->m_Attachments.back().m_PixelRectangle.m_Pixels.resize(
                resource->m_Attachments.back().m_PixelRectangle.m_Width * 
                resource->m_Attachments.back().m_PixelRectangle.m_Height * 
                resource->m_Attachments.back().m_PixelRectangle.m_Channels
                );

            DIRECT_CALL_CHK(glReadBuffer)(GL_COLOR_ATTACHMENT0 + i);
            DIRECT_CALL_CHK(glReadPixels)(0, 0, resource->m_Attachments.back().m_PixelRectangle.m_Width,
                resource->m_Attachments.back().m_PixelRectangle.m_Height,
                format, GL_UNSIGNED_BYTE, &resource->m_Attachments.back().m_PixelRectangle.m_Pixels[0]);
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

    // Forgot about:
    //GL_EXTENSIONS
    //GL_SHADING_LANGUAGE_VERSION
    //GL_SHADING_LANGUAGE_VERSION
    
    resource->m_Renderer = (const char*)DIRECT_CALL_CHK(glGetString)(GL_RENDERER);
    resource->m_Vendor   = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VENDOR);
    resource->m_Version  = (const char*)DIRECT_CALL_CHK(glGetString)(GL_VERSION);

    resource->m_hasNVXGPUMemoryInfo = m_HasNVXMemoryInfo;
    if (resource->m_hasNVXGPUMemoryInfo = m_HasNVXMemoryInfo) {
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


boost::shared_ptr<DGLResource> GLContext::queryState(GLuint name) {

    DGLResourceState* resource;
    boost::shared_ptr<DGLResource> ret (resource = new DGLResourceState);

#define GET_STATE(NAME, VALUE, LENGTH) {                            \
        DGLResourceState::StateItem item;                           \
        item.m_Name = NAME;                                         \
        std::vector<GLdouble> val(LENGTH, 0);                       \
        DIRECT_CALL_CHK(glGetDoublev)(VALUE, &val[0]);              \
        if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
            item.m_Values.resize(val.size());                       \
            for (int i = 0; i < LENGTH; i++) {                      \
                item.m_Values[i] = val[i];                          \
            }                                                       \
        }                                                           \
    resource->m_Items.push_back(item);                              \
}


#define STATE_INTEGERV_BASE(NAME, VALUE, LENGTH) {              \
    DGLResourceState::StateItem item;                           \
    item.m_Name = NAME;                                         \
    std::vector<GLint> val(LENGTH, 0);                          \
    DIRECT_CALL_CHK(glGetIntegerv)(VALUE, &val[0]);             \
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
        item.m_Values.resize(val.size());                       \
        for (int i = 0; i < LENGTH; i++) {                      \
            item.m_Values[i] = val[i];                          \
        }                                                       \
    }                                                           \
    resource->m_Items.push_back(item);                          \
}

#define STATE_INTEGERV(NAME, LENGTH) STATE_INTEGERV_BASE(#NAME, NAME, LENGTH)
#define STATE_INTEGER64V(NAME, LENGTH) STATE_INTEGERV_BASE(#NAME, NAME, LENGTH)
#define STATE_INTEGERENUMV(NAME, LENGTH) STATE_INTEGERV_BASE(#NAME, NAME, LENGTH)


#define STATE_FLOATV(NAME, LENGTH) {              \
    DGLResourceState::StateItem item;                            \
    item.m_Name = #NAME;                                         \
    std::vector<GLfloat> val(LENGTH, 0);                           \
    DIRECT_CALL_CHK(glGetFloatv)(NAME, &val[0]);               \
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
        item.m_Values.resize(val.size());                       \
        for (int i = 0; i < LENGTH; i++) {                      \
            item.m_Values[i] = val[i];                          \
        }                                                       \
    }                                                           \
        resource->m_Items.push_back(item);                      \
}


#define STATE_DOUBLEV(NAME, LENGTH) {              \
    DGLResourceState::StateItem item;                            \
    item.m_Name = #NAME;                                         \
    std::vector<GLdouble> val(LENGTH, 0);                           \
    DIRECT_CALL_CHK(glGetDoublev)(NAME, &val[0]);               \
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
        item.m_Values.resize(val.size());                       \
        for (int i = 0; i < LENGTH; i++) {                      \
            item.m_Values[i] = val[i];                          \
        }                                                       \
    }                                                           \
    resource->m_Items.push_back(item);                      \
}

#define STATE_BOOLEANV(NAME, LENGTH) {              \
    DGLResourceState::StateItem item;                            \
    item.m_Name = #NAME;                                         \
    std::vector<GLboolean> val(LENGTH, 0);                           \
    DIRECT_CALL_CHK(glGetBooleanv)(NAME, &val[0]);               \
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
    item.m_Values.resize(val.size());                       \
        for (int i = 0; i < LENGTH; i++) {                      \
            item.m_Values[i] = val[i];                          \
        }                                                       \
    }                                                           \
    resource->m_Items.push_back(item);                      \
}

#define STATE_ISENABLED(NAME) {              \
    DGLResourceState::StateItem item;                            \
    item.m_Name = #NAME;                                         \
    GLboolean val =  DIRECT_CALL_CHK(glIsEnabled)(NAME);               \
    if (DIRECT_CALL_CHK(glGetError)() == GL_NO_ERROR) {         \
        item.m_Values.resize(1, val);                       \
    }                                                           \
    resource->m_Items.push_back(item);                      \
}


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
    STATE_DOUBLEV(GL_DEPTH_RANGE, 2);
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
    //STATE_INTEGERV(GL_COMPRESSED_TEXTURE_FORMATS_4__Z+);
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
        if (strcmp("GL_NVX_gpu_memory_info", ext) == 0)
            m_HasNVXMemoryInfo = true;
    }
    if (debugOutputSupported) {
        DIRECT_CALL_CHK(glEnable)(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        DIRECT_CALL_CHK(glDebugMessageCallbackARB)(debugOutputCallback, this);
    }
}

NativeSurface::NativeSurface(uint32_t id):m_Id(id) {
    HDC hdc = reinterpret_cast<HDC>(id);
    int i = DIRECT_CALL_CHK(wglGetPixelFormat)(hdc);
    PIXELFORMATDESCRIPTOR  pfd;
    DIRECT_CALL_CHK(wglDescribePixelFormat)(hdc, i, 
        sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    m_DoubleBuffered = (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;
    m_Stereo = (pfd.dwFlags & PFD_STEREO) != 0;
    m_RGBASizes[0] = pfd.cRedBits;
    m_RGBASizes[1] = pfd.cGreenBits;
    m_RGBASizes[2] = pfd.cBlueBits;
    m_RGBASizes[3] = pfd.cAlphaBits;
    m_StencilSize = pfd.cStencilBits;
    m_DepthSize = pfd.cDepthBits;
    RECT rc;
    GetClientRect(WindowFromDC(hdc), &rc);
    m_Width = rc.right - rc.left;
    m_Height = rc.bottom - rc.top;
}


uint32_t NativeSurface::getId() {
    return m_Id;
}

bool NativeSurface::isDoubleBuffered() {
    return m_DoubleBuffered;
}

bool NativeSurface::isStereo() {
    return m_Stereo;
}

int* NativeSurface::getRGBASizes() {
    return m_RGBASizes;
}

int NativeSurface::getStencilSize() {
    return m_StencilSize;
}

int NativeSurface::getDepthSize() {
    return m_DepthSize;
}


int NativeSurface::getWidth() {
    return m_Width;
}

int NativeSurface::getHeight() {
    return m_Height;
}


} //namespace dglState