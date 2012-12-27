#ifndef GL_STATE_H
#define GL_STATE_H

#include <DGLCommon/gl-types.h>
#include <DGLNet/message.h>

#include <map>
#include <queue>
#include <boost/shared_ptr.hpp>

namespace dglState {

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

class GLShaderObj: public GLObj {
public:
    GLShaderObj(GLuint name);
    GLShaderObj() {}
    void markDeleted();

    void setSources(const std::vector<std::string>&);
    void setCompilationStatus(const std::string&, GLint compileStatus);
    void setTarget(GLenum target);

    const std::vector<std::string>& getSources() const;
    GLenum getTarget() const;
    const std::pair<std::string, GLint>& getCompileStatus() const;

private:
    bool m_Deleted;
    std::vector<std::string> m_Sources;
    std::pair<std::string, GLint> m_CompileStatus;
    GLenum m_Target;
};

class GLProgramObj: public GLObj {
public:
    GLProgramObj(GLuint name);
    GLProgramObj() {}
    void use(bool inUse); 
    bool mayDelete(); 
    void markDeleted();
    void attachShader(GLShaderObj*);
    void detachShader(GLShaderObj*);
    std::set<GLShaderObj*>& getAttachedShaders();

private:
    int m_InUse;
    bool m_Deleted;
    std::set<GLShaderObj*> m_AttachedShaders;
};

class GLFBObj: public GLObj {
public:
    GLFBObj(GLuint name);
    GLFBObj() {}
    void setTarget(GLenum);
    GLenum getTarget();
private:
    GLenum m_Target;
};

//Native platform interface surface. HDC on windows
class NativeSurface {
public:
    NativeSurface(uint32_t);
    uint32_t getId();
    bool isDoubleBuffered();
    bool isStereo();

    int* getRGBASizes();
    int getStencilSize();
    int getDepthSize();

    int getWidth(); 
    int getHeight(); 
private:
    uint32_t m_Id;
    int m_Width, m_Height;
    bool m_Stereo, m_DoubleBuffered;
    int m_RGBASizes[4];
    int m_DepthSize, m_StencilSize;

};

class GLContext {
public:
    GLContext(uint32_t id);
    bool m_deleted;
    std::map<GLuint, GLTextureObj> m_Textures;
    std::map<GLuint, GLBufferObj> m_Buffers;
    std::map<GLuint, GLProgramObj> m_Programs;
    std::map<GLuint, GLShaderObj> m_Shaders;
    std::map<GLuint, GLFBObj> m_FBOs;

    dglnet::ContextReport describe();

    void use(bool);
    bool lazyDelete();
    bool isDeleted();

    NativeSurface* getNativeSurface();
    void setNativeSurface(NativeSurface*);

    GLTextureObj* ensureTexture(GLuint name);
    void deleteTexture(GLuint name);
    GLBufferObj* ensureBuffer(GLuint name);
    void deleteBuffer(GLuint name);
    GLFBObj* ensureFBO(GLuint name);
    void deleteFBO(GLuint name);
    GLProgramObj* ensureProgram(GLuint name);
    void deleteProgram(GLuint name);
    GLShaderObj* ensureShader(GLuint name);
    void markShaderDeleted(GLuint name);

    boost::shared_ptr<DGLResource> queryTexture(GLuint name);
    boost::shared_ptr<DGLResource> queryBuffer(GLuint name);
    boost::shared_ptr<DGLResource> queryFramebuffer(GLuint bufferEnum);
    boost::shared_ptr<DGLResource> queryFBO(GLuint name);
    boost::shared_ptr<DGLResource> queryShader(GLuint name);
    boost::shared_ptr<DGLResource> queryProgram(GLuint name);
    boost::shared_ptr<DGLResource> queryGPU(GLuint name);
    boost::shared_ptr<DGLResource> queryState(GLuint name);

    int32_t getId();

    std::pair<bool, GLenum> getPokedError();
    GLenum peekError();

    void setDebugOutput(const std::string& message);
    bool hasDebugOutput();
    const std::string& popDebugOutput();


    void startQuery();
    bool endQuery(std::string& message);

    /**
     * Imemdiate mode setter - must be set, when betweek glBegin()/glEnd(), otherwise spurious GL errors will happen
     * No query will be emitted when in immediate mode
     */
    void setImmediateMode(bool);

private:
    void queryCheckError();
    

    void firstUse();       

    /**
     * WGL or other native API context ID
     */
    int32_t m_Id;

    bool m_InUse, m_Deleted, m_EverUsed;

    /**
     * Handle to native surface (drawable)
     */
    NativeSurface* m_NativeSurface;

    /**
     * Queue for errors poked from glGetError(), not yet delivered to application
     */
    std::queue<GLenum> m_PokedErrorQueue;


    /**
     * Set if NVX_gpu_memory_info is present
     */
    bool m_HasNVXMemoryInfo;

    /**
     * Set to if pending message from debug output is present
     */
    bool m_HasDebugOutput;

    /**
     * Pending message from debug output
     */
    std::string m_DebugOutput;

    /**
     * Set to true if betweek glBegin() and glEnd() 
     */
    bool m_InImmediateMode;

    /**
     * Get state element (using glGetIntegerv)
     */
    DGLResourceState::StateItem getStateIntegerv(const char* name, GLenum value, size_t length);

    /**
    * Get state element (using glGetInteger64v)
    */
    DGLResourceState::StateItem getStateInteger64v(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetFloatv)
     */
    DGLResourceState::StateItem getStateFloatv(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetDoublev)
     */
    DGLResourceState::StateItem getStateDoublev(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetBooleanv)
     */
    DGLResourceState::StateItem getStateBooleanv(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glIsEnabled)
     */
    DGLResourceState::StateItem getStateIsEnabled(const char* name, GLenum value);
};

} //namespace
#endif


DGLResourceState::StateItem getState(const char* name, GLenum value, size_t length);