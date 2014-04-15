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

#ifndef GL_AUX_CONTEXT_H
#define GL_AUX_CONTEXT_H

#include <DGLCommon/gl-types.h>
#include <map>
#include <memory>
#include <vector>

class DGLDisplayState;

namespace dglState {

class GLContext;

class GLAuxContext;

class GLAuxContextSession {
   public:
    ~GLAuxContextSession();
    GLAuxContextSession(GLAuxContext*);
    
    void dispose();

   private:
    GLAuxContext* m_ctx;
};

class GLAuxContextSurfaceBase {
   protected:
    GLAuxContextSurfaceBase(const DGLDisplayState* display);
   public:
    virtual ~GLAuxContextSurfaceBase() {}
    opaque_id_t getId() const;

   protected:
    opaque_id_t m_DisplayId;
    opaque_id_t m_Id;
};

class GLAuxEGLContextSurface: public GLAuxContextSurfaceBase  {
public:
    GLAuxEGLContextSurface(const DGLDisplayState* display, opaque_id_t pixfmt);
    ~GLAuxEGLContextSurface();
};

class GLAuxWGLContextSurface: public GLAuxContextSurfaceBase  {
public:
    GLAuxWGLContextSurface(const DGLDisplayState* display, opaque_id_t pixfmt);
    ~GLAuxWGLContextSurface();
};

/*
class GLAuxGLXContextSurface: public GLAuxContextSurfaceBase  {
public:
    GLAuxGLXContextSurface(const DGLDisplayState* display, opaque_id_t pixfmt);
    ~GLAuxGLXContextSurface();
};
*/

class GLAuxContext {
   public:
    GLAuxContext(const GLContext*);
    ~GLAuxContext();

    GLAuxContextSession makeCurrent();

    class GLQueries {
       public:
        GLQueries(GLAuxContext*);

        void setupInitialState();

        void auxDrawTexture(GLuint name, GLenum target, GLint level, GLint layer, GLint face,
                            GLenum textureBaseFormat, GLenum renderableFormat, int width, int height);

        void auxGetBufferData(GLuint name, std::vector<char>& ret);

       private:

        static const int BufferGetterChunkSize;

        GLuint getTextureShaderProgram(GLenum target, GLenum textureBaseFormat);

        GLuint compileShader(GLenum type, const char* src);
        void linkProgram(GLuint program);


        GLuint fbo, vao, vbo, rtt, vshobjTexture;
        std::map<std::string, GLuint> programsTexture;
        GLuint programGetBuffer;

        bool m_InitialState;

        GLAuxContext* m_AuxCtx;

    } queries;

   private:
    opaque_id_t choosePixelFormat(opaque_id_t preferred, opaque_id_t displayId);

    void doRefCurrent();
    bool doUnrefCurrent();

    opaque_id_t m_Id, m_PixelFormat;
    const GLContext* m_Parrent;
    int m_MakeCurrentRef;

    std::shared_ptr<GLAuxContextSurfaceBase> m_AuxSurface;

    friend class GLAuxContextSession;
};

}    // namespace
#endif
