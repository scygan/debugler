#ifndef GL_STATE_H
#define GL_STATE_H

#include <DGLCommon/gl-types.h>
#include <DGLNet/message.h>

#include <map>

namespace dglstate {

class GLObj {
public:
    GLObj();
    GLObj(GLuint name);
    GLuint getName();
private:
    GLuint m_Name;
};

class GLTextureObj: public GLObj {
public:
    GLTextureObj(GLuint name);
    GLTextureObj() {}
    void setTarget(GLenum);
    GLenum getTarget();
private:
    GLenum m_Target;
};

class GLBufferObj: public GLObj {
public:
    GLBufferObj(GLuint name);
    void setTarget(GLenum);
    GLenum getTarget();
    GLBufferObj() {}
private:
    GLenum m_Target;
};

class GLProgramObj: public GLObj {
public:
    GLProgramObj(GLuint name);
    GLProgramObj() {}
    void use(bool inUse); 
    bool mayDelete(); 
    void markDeleted();

private:
    int m_InUse;
    bool m_Deleted;
};

class GLContext {
public:
    GLContext(uint32_t id);
    bool m_deleted;
    std::map<GLuint, GLTextureObj> m_Textures;
    std::map<GLuint, GLBufferObj> m_Buffers;
    std::map<GLuint, GLProgramObj> m_Programs;

    dglnet::ContextReport describe();

    void use(bool);
    bool lazyDelete();
    bool isDeleted();

    GLTextureObj* ensureTexture(GLuint name);
    void deleteTexture(GLuint name);
    GLBufferObj* ensureBuffer(GLuint name);
    void deleteBuffer(GLuint name);

    void queryTexture(GLuint name, dglnet::TextureMessage& ret);
    void queryBuffer(GLuint name, dglnet::BufferMessage& ret);


    GLProgramObj* ensureProgram(GLuint name);
    void deleteProgram(GLuint name);

    int32_t getId();

private:
    int32_t m_Id;
    bool m_InUse, m_Deleted;
};

} //namespace
#endif