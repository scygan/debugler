#include "gl-state.h"

#include <boost/make_shared.hpp>

#include "pointers.h"

namespace dglstate {

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

GLContext::GLContext(uint32_t id):m_Id(id), m_InUse(false), m_Deleted(false) {}

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

void GLContext::queryTexture(GLuint name, dglnet::TextureMessage& ret) {
    
    ret.m_TextureName = name;

    //check if GL knows about a texture
    if (DIRECT_CALL(glIsTexture)(name) != GL_TRUE) {
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
    GLint pbo = 0;
    DIRECT_CALL(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &pbo);
    if (pbo) {
        DIRECT_CALL(glBindBuffer)(GL_PIXEL_PACK_BUFFER, 0);
    }

    //rebind texture, so we can access it
    GLint lastTexture = 0;
    switch (tex->getTarget()) {
        case GL_TEXTURE_2D:
            DIRECT_CALL(glGetIntegerv)(GL_TEXTURE_BINDING_2D, &lastTexture);
            break;
        default:
            assert(0);
    }
    if (lastTexture != tex->getName()) {
        DIRECT_CALL(glBindTexture)(tex->getTarget(), tex->getName());
    }

    //dump and set pixel store state
    GLenum pixelStoreTargets[] = {GL_PACK_SWAP_BYTES, GL_PACK_LSB_FIRST, GL_PACK_ROW_LENGTH, GL_PACK_IMAGE_HEIGHT, GL_PACK_SKIP_ROWS,
        GL_PACK_SKIP_PIXELS, GL_PACK_SKIP_IMAGES, GL_PACK_ALIGNMENT};
    GLint dglPixelStoreStare[sizeof(pixelStoreTargets)/sizeof(pixelStoreTargets[1])] =
        {GL_FALSE, GL_FALSE, 0, 0, 0, 0, 0, 1 };
    GLint pixelStoreState[sizeof(pixelStoreTargets)/sizeof(pixelStoreTargets[1])];
    for (int i = 0; i < sizeof(pixelStoreTargets)/sizeof(pixelStoreTargets[1]); i++) {
        DIRECT_CALL(glGetIntegerv)(pixelStoreTargets[i], &pixelStoreState[i]);
        DIRECT_CALL(glPixelStorei)(pixelStoreTargets[i], dglPixelStoreStare[i]);
    }

    {
        int level = 0;
        dglnet::TextureLevel texLevel;
        DIRECT_CALL(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &texLevel.m_Width);
        DIRECT_CALL(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_HEIGHT, &texLevel.m_Height);
        GLint alpha;
        GLenum format;
        DIRECT_CALL(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_ALPHA_SIZE, &alpha);
        if (alpha) {
            texLevel.m_Channels = 4;
            format = GL_RGBA;
        } else {
            texLevel.m_Channels = 3;
            format = GL_RGB;
        }
         
        texLevel.m_Pixels.resize(texLevel.m_Width * texLevel.m_Height * texLevel.m_Channels);
        DIRECT_CALL(glGetTexImage)(tex->getTarget(), level, format, GL_FLOAT, &texLevel.m_Pixels[0]);
        ret.m_Levels.push_back(texLevel);
    }

    //restore state
    for (int i = 0; i < sizeof(pixelStoreTargets)/sizeof(pixelStoreTargets[1]); i++) {
        DIRECT_CALL(glPixelStorei)(pixelStoreTargets[i], pixelStoreState[i]);
    }
    if (pbo) {
        DIRECT_CALL(glBindBuffer)(GL_PIXEL_PACK_BUFFER, pbo);
    }
    if (lastTexture != tex->getName()) {
        DIRECT_CALL(glBindTexture)(tex->getTarget(), lastTexture);
    }
}

void GLContext::queryBuffer(GLuint name, dglnet::BufferMessage& ret) {

    ret.m_BufferName = name;

    //check if GL knows about a texture
    if (DIRECT_CALL(glIsBuffer)(name) != GL_TRUE) {
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
            DIRECT_CALL(glGetIntegerv)(GL_ARRAY_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_ATOMIC_COUNTER_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_ATOMIC_COUNTER_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_COPY_READ_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_COPY_READ_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_COPY_WRITE_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_COPY_WRITE_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_DRAW_INDIRECT_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_DRAW_INDIRECT_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_DISPATCH_INDIRECT_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_PIXEL_PACK_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_PIXEL_PACK_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_PIXEL_UNPACK_BUFFER:
            DIRECT_CALL(glGetIntegerv)( GL_PIXEL_UNPACK_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_SHADER_STORAGE_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_SHADER_STORAGE_BUFFER_BINDING, &lastBuffer);
            break;
        /*case GL_TEXTURE_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_NO_IDEA_WHAT_SHOULD_BE_HERE, &lastBuffer);
            break;*/
        case GL_TRANSFORM_FEEDBACK_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &lastBuffer);
            break;
        case GL_UNIFORM_BUFFER:
            DIRECT_CALL(glGetIntegerv)(GL_UNIFORM_BUFFER_BINDING, &lastBuffer);
            break;       
    default:
        ret.error("Buffer target is not supported");
        return;
    }
    if (lastBuffer != buff->getName()) {
        DIRECT_CALL(glBindBuffer)(buff->getTarget(), buff->getName());
    }

    GLint mapped; 
    DIRECT_CALL(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_MAPPED, &mapped);
    if (mapped == GL_TRUE) {
        ret.error("Buffer is currently mapped, cannot access data.");
    } else {
        GLint size = 0; 
        DIRECT_CALL(glGetBufferParameteriv)(buff->getTarget(), GL_BUFFER_SIZE, &size);
        if (!size) {
            ret.error("Buffer empty (GL_BUFFER_SIZE is 0)");
        } else {
            ret.m_Data.resize(size);
            DIRECT_CALL(glGetBufferSubData)(buff->getTarget(), 0, size, &ret.m_Data[0]);
        }
    }

    //restore state
    if (lastBuffer != buff->getName()) {
        DIRECT_CALL(glBindBuffer)(buff->getTarget(), lastBuffer);
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

} //namespace dglstate