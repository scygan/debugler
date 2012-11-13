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

void GLContext::ensureBuffer(GLuint name) {
    if (m_Buffers.find(name) == m_Buffers.end())
        m_Buffers[name] = GLBufferObj(name);
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


    {
        int level = 0;
        dglnet::TextureLevel texLevel;
        DIRECT_CALL(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_WIDTH, &texLevel.m_Width);
        DIRECT_CALL(glGetTexLevelParameteriv)(tex->getTarget(), level, GL_TEXTURE_HEIGHT, &texLevel.m_Height);
        texLevel.m_Channels = 4;
        texLevel.m_Pixels.resize(texLevel.m_Width * texLevel.m_Height * texLevel.m_Channels, 0.25f);

        //DIRECT_CALL(glGetTexImage)(tex->getTarget(), level, GL_RGBA, GL_FLOAT, &texLevel.m_Pixels[0]);
        ret.m_Levels.push_back(texLevel);
    }

    //restore state
    if (pbo) {
        DIRECT_CALL(glBindBuffer)(GL_PIXEL_PACK_BUFFER, pbo);
    }
    if (lastTexture != tex->getName()) {
        DIRECT_CALL(glBindTexture)(tex->getTarget(), lastTexture);
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