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
#include <map>
#include <vector>
#include <string>

namespace dglState {

class GLContext;

class GLObj {
   public:
    GLObj();
    GLObj(GLuint name);
    GLuint getName() const;
    
    void setTarget(GLenum);
    GLenum getTarget() const;

   private:
    GLuint m_Name;
    GLenum m_Target;
};

class GLTextureObj : public GLObj {
   public:
    /**
     * Ctor
     */
    GLTextureObj(GLuint name);

    GLTextureObj() {}

    /** 
     * Set texture level image params (called on glTexImage)
     */
    void setTexImage(GLuint level, GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat, GLenum format, GLenum type);

    /** 
     * Set texture params (called on glTexStorage)
     */
    void setTexStorage(GLuint levels, GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * Get texture format and sample count
     */
    void getFormat(GLContext* ctx, int level, GLenum levelTarget, GLint& retInternalFormat, GLint& retSamples) const;

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
   
    /** 
     * Level parameters
     */
    std::vector<GLTextureLevel> m_Levels;
};

class GLBufferObj : public GLObj {
   public:
    GLBufferObj(GLuint name);
    GLBufferObj() {}
};

class GLObjectNameSpaces;

class GLShaderObj : public GLObj {
   public:
     
    struct GLShaderObjCreateData {
        inline GLShaderObjCreateData(GLObjectNameSpaces* parrent, bool arbApi): m_Parrent(parrent), m_ArbApi(arbApi) {}
        GLObjectNameSpaces* m_Parrent;
        bool m_ArbApi;
    };

    GLShaderObj(GLuint name, GLShaderObjCreateData createData);
    void deleteCalled();
    void incRefCount();
    void decRefCount();
    int getRefCount();

    void createCalled(GLenum target);

    GLint queryCompilationStatus() const;
    std::string queryCompilationInfoLog() const;

    void shaderSourceCalled();

    std::string querySource();

    void editSource(const std::string& source);
    void resetSourceToOrig();

   private:
    void deleteSelfIfNeeded();

    bool m_DeleteCalled;
    std::string m_OrigSource;
    bool m_arbApi;
    int m_RefCount;

    GLObjectNameSpaces* m_Parrent;
};

class GLProgramObj : public GLObj {
   public:
    GLProgramObj(GLuint name, bool arbApi);
    GLProgramObj() {}
    ~GLProgramObj();
    void setInUse(bool inUse);
    bool mayDelete();
    void markDeleted();
    void attachShader(GLShaderObj*);
    void detachShader(GLShaderObj*);
    std::set<GLShaderObj*>& getAttachedShaders();

    void forceLink();

    void setEmbeddedSSOSource(GLsizei count, const char* const* strings);
    inline const std::string& getEmbeddedSSOSource() { return m_EmbeddedSSOSource; }

   private:
    bool m_InUse;
    bool m_Deleted;
    bool m_arbApi;
    std::set<GLShaderObj*> m_AttachedShaders;

    /** 
     * GLSL source for embedded SSO (glCreateShaderProgram)
     */
    std::string m_EmbeddedSSOSource;
};

class GLFBObj : public GLObj {
   public:
    GLFBObj(GLuint name);
    GLFBObj() {}
};

class GLRenderbufferObj : public GLObj {
   public:
    GLRenderbufferObj(GLuint name);
    GLRenderbufferObj() {}
};

}    // namespace
#endif
