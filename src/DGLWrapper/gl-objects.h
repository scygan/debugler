/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include <DGLCommon/gl-headers.h>

#include <set>
#include <vector>
#include <string>

namespace dglState {

class GLObj {
   public:
    GLObj();
    GLObj(GLuint name);
    GLuint getName() const;

   private:
    GLuint m_Name;
};

class GLTextureObj : public GLObj {
   public:
    /**
     * Ctor
     */
    GLTextureObj(GLuint name);

    GLTextureObj() {}

    /**
     * Set texture target (it is detected usually on glBindTexture())
     */
    void setTarget(GLenum);

    /** 
     * Set texture level image params (called on glTexImage)
     */
    void setTexImage(GLuint level, GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat, GLenum format, GLenum type);

    /** 
     * Set texture params (called on glTexStorage)
     */
    void setTexStorage(GLuint levels, GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * Get texture target
     */
    GLenum getTarget() const;

    /**
     * Get  target of texture level. 
     *
     * Face is required argument for cubemaps.
     */
    GLenum getTextureLevelTarget(int face) const;

    /**
     * Class describing level parameters 
     */
    class GLTextureLevel {
    public:
        GLTextureLevel();
        GLTextureLevel(GLenum requestedInternalFormat, GLenum requestedDataType, GLsizei width, GLsizei height, GLsizei depth);

        GLenum m_RequestedInternalFormat;
        GLenum m_RequestedDataType;
        GLsizei m_Width, m_Height, m_Depth;
    };

    /**
     * Getter for requested texture level parameters
     */
    const GLTextureLevel* getRequestedLevel(GLint level) const;
   
   private:
    /**
     * Texture target. Must be cached here - not retrievable by GL API
     */
    GLenum m_Target;

    /** 
     * Level parameters
     */
    std::vector<GLTextureLevel> m_Levels;
};

class GLBufferObj : public GLObj {
   public:
    GLBufferObj(GLuint name);
    void setTarget(GLenum);
    GLenum getTarget();
    GLBufferObj() {}

   private:
    GLenum m_Target;
};

class GLShaderObj : public GLObj {
   public:
    GLShaderObj(GLuint name, bool arbApi);
    GLShaderObj() {}
    void deleteCalled();
    void incRefCount();
    void decRefCount();
    int getRefCount();

    void createCalled(GLenum target);
    GLenum getTarget() const;

    GLint queryCompilationStatus() const;
    std::string queryCompilationInfoLog() const;

    void shaderSourceCalled();

    const std::string& querySource();
    bool isDeleted() const;

    void editSource(const std::string& source);
    void resetSourceToOrig();

   private:
    void mayDelete();

    bool m_Deleted;
    bool m_DeleteCalled;
    std::string m_Source, m_OrigSource;
    GLenum m_Target;
    bool m_arbApi;
    int m_RefCount;
};

class GLProgramObj : public GLObj {
   public:
    GLProgramObj(GLuint name, bool arbApi);
    GLProgramObj() {}
    ~GLProgramObj();
    void use(bool inUse);
    bool mayDelete();
    void markDeleted();
    void attachShader(GLShaderObj*);
    void detachShader(GLShaderObj*);
    std::set<GLShaderObj*>& getAttachedShaders();

    void forceLink();

   private:
    bool m_InUse;
    bool m_Deleted;
    bool m_arbApi;
    std::set<GLShaderObj*> m_AttachedShaders;
};

class GLFBObj : public GLObj {
   public:
    GLFBObj(GLuint name);
    GLFBObj() {}
    void setTarget(GLenum);
    GLenum getTarget();

   private:
    GLenum m_Target;
};

}    // namespace
#endif
