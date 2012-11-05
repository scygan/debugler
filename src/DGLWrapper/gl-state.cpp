#include "gl-state.h"

#include <boost/make_shared.hpp>

namespace dglstate {

GLObj::GLObj():m_Name(0) {}

GLObj::GLObj(GLuint name):m_Name(name) {}

GLuint GLObj::getName() { return m_Name; }


GLTextureObj::GLTextureObj(GLuint name):GLObj(name) {}


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

void GLContext::ensureTexture(GLuint name) {
    if (m_Textures.find(name) == m_Textures.end())
        m_Textures[name] = GLTextureObj(name);
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