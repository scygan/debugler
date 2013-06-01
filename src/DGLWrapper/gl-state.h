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


#ifndef GL_STATE_H
#define GL_STATE_H

#include <map>
#include <queue>
#include <boost/shared_ptr.hpp>

#include <DGLCommon/gl-types.h>
#include <DGLNet/protocol/message.h>
#include <DGLNet/protocol/resource.h>

class DGLDisplayState;

namespace dglState {

class GLObj {
public:
    GLObj();
    GLObj(GLuint name);
    GLuint getName() const;
private:
    GLuint m_Name;
};

class GLTextureObj: public GLObj {
public:
    /**
     * Ctor
     */
    GLTextureObj(GLuint name);
    
    GLTextureObj() {}

    /**
     * Get texture target (it is detected usually on glBindTexture())
     */
    void setTarget(GLenum);

    /**
     * Get texture target
     */
    GLenum getTarget();
    
    /**
     * Get texture level target from texture target and face
     */
    GLenum getTextureLevelTarget(int face);
private:

    /**
     * Texture target. Must be cached here - not retrievable by GL API
     */
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
    GLShaderObj(GLuint name, bool arbApi);
    GLShaderObj() {}
    void deleteCalled();
    void incRefCount();
    void decRefCount();
    int getRefCount();

    void createCalled(GLenum target);
    GLenum getTarget() const;
    
    //bool useArbApi() const;

    GLint queryCompilationStatus() const;
    std::string queryCompilationInfoLog() const;
    void cacheSources();
    std::string querySources();
    bool isDeleted() const;

    void editSource(const std::string& source);


private:

    void mayDelete();

    bool m_Deleted;
    bool m_DeleteCalled;
    std::string m_LastSources;
    GLenum m_Target;
    bool m_arbApi;
    int m_RefCount;
};

class GLProgramObj: public GLObj {
public:
    GLProgramObj(GLuint name);
    GLProgramObj() {}
    ~GLProgramObj();
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

class GLContextVersion {
public:
    enum Type {
        DT, 
        ES,
        UNSUPPORTED,
    };
    GLContextVersion(Type type);

    bool check(Type type, int majorVersion = 0, int minorVersion = 0);

private:
    void fill();
    bool m_Filled;
    int m_MajorVersion;
    int m_MinorVersion;
    Type m_Type;
};

class NativeSurfaceBase;

class GLContext {
public:
    GLContext(GLContextVersion version, opaque_id_t id);
    std::map<GLuint, GLTextureObj> m_Textures;
    std::map<GLuint, GLBufferObj> m_Buffers;
    std::map<GLuint, GLProgramObj> m_Programs;
    std::map<GLuint, GLShaderObj> m_Shaders;
    std::map<GLuint, GLFBObj> m_FBOs;

    dglnet::message::BreakedCall::ContextReport describe();

    NativeSurfaceBase* getNativeReadSurface();
    void setNativeReadSurface(NativeSurfaceBase*);

    GLTextureObj* ensureTexture(GLuint name);
    void deleteTexture(GLuint name);
    GLBufferObj* ensureBuffer(GLuint name);
    void deleteBuffer(GLuint name);
    GLFBObj* ensureFBO(GLuint name);
    void deleteFBO(GLuint name);
    GLProgramObj* ensureProgram(GLuint name);
    GLProgramObj* findProgram(GLuint name);
    void deleteProgram(GLuint name);
    GLShaderObj* ensureShader(GLuint name, bool fromArbAPI);
    GLShaderObj* findShader(GLuint name);

    boost::shared_ptr<dglnet::DGLResource> queryTexture(gl_t name);
    boost::shared_ptr<dglnet::DGLResource> queryBuffer(gl_t name);
    boost::shared_ptr<dglnet::DGLResource> queryFramebuffer(gl_t bufferEnum);
    boost::shared_ptr<dglnet::DGLResource> queryFBO(gl_t name);
    boost::shared_ptr<dglnet::DGLResource> queryShader(gl_t name);
    boost::shared_ptr<dglnet::DGLResource> queryProgram(gl_t name);
    boost::shared_ptr<dglnet::DGLResource> queryGPU();
    boost::shared_ptr<dglnet::DGLResource> queryState(gl_t name);

    opaque_id_t getId();

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

    /**
     * Called to tell ctx when if is bound to current thread
     */
    void bound();

    /**
     * Called to tell ctx when it is lazy deleted.
     * returns true, if no longer used
     */
    bool markForDeletionMayDelete();

    /**
     * Called to tell ctx when if is unbound from current thread.
     * returns true, if context was marked for deletion and no longer used
     */
    bool unboundMayDelete();

    /**
     * Getter for context version
     */
    GLContextVersion& getVersion();

private:
    void queryCheckError();
    

    void firstUse();

    /**
     * API version of underlying GL context
     */
    GLContextVersion m_Version;

    /**
     * WGL or other native API context ID
     */
    opaque_id_t m_Id;

    /**
     * Handle to native surface (drawable)
     */
    NativeSurfaceBase* m_NativeReadSurface;

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
    dglnet::resource::DGLResourceState::StateItem getStateIntegerv(const char* name, GLenum value, size_t length);

    /**
    * Get state element (using glGetInteger64v)
    */
    dglnet::resource::DGLResourceState::StateItem getStateInteger64v(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetFloatv)
     */
    dglnet::resource::DGLResourceState::StateItem getStateFloatv(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetDoublev)
     */
    dglnet::resource::DGLResourceState::StateItem getStateDoublev(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glGetBooleanv)
     */
    dglnet::resource::DGLResourceState::StateItem getStateBooleanv(const char* name, GLenum value, size_t length);

    /**
     * Get state element (using glIsEnabled)
     */
    dglnet::resource::DGLResourceState::StateItem getStateIsEnabled(const char* name, GLenum value);

    /**
     * Get texture currently bound to given target
     */
    GLuint getBoundTexture(GLenum target);

    /**
     * True if ctx was ever bound, false otherwise
     */
    bool m_EverBound;

    /** 
     * Number of threads this context is bound to
     */
    int m_RefCount;

    /** 
     * True if deletion is pending
     */
    bool m_ToBeDeleted;
};

} //namespace
#endif