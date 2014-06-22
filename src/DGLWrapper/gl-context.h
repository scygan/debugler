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

#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include "gl-shadowstate.h"
#include "gl-objects.h"
#include "gl-object-namespace.h"
#include "gl-statesetters.h"

#include <DGLCommon/gl-types.h>
#include <DGLCommon/gl-entrypoints.h>
#include <DGLNet/protocol/msgutils.h>

#include <vector>
#include <queue>
#include <map>
#include <memory>


class DGLDisplayState;

namespace dglState {

class GLContextVersion {
   public:
    enum class Type {
        DT,
        ES,
        UNSUPPORTED,
    };
    GLContextVersion(Type type, int majorVersion = 0, int minorVersion = 0);

    bool check(Type type, int majorVersion = 0, int minorVersion = 0) const;
    void initialize(const char* cVersion);

    int getMajor() const;

    int getNeededApiLibraries(const DGLDisplayState* display);

   private:
    bool m_Initialized;
    int m_MajorVersion;
    int m_MinorVersion;
    Type m_Type;
};

class NativeSurfaceBase;

class GLContextCreationData {
   public:
    GLContextCreationData();
    GLContextCreationData(Entrypoint entryp, opaque_id_t pixelFormat,
                          const std::vector<gl_t>& attribs);
    Entrypoint getEntryPoint() const;
    opaque_id_t getPixelFormat() const;
    const std::vector<gl_t>& getAttribs() const;

   private:
    Entrypoint m_Entrypoint;
    opaque_id_t m_pixelFormat;
    std::vector<gl_t> m_attribs;
};

class GLAuxContext;

class GLContext {
   public:
    GLContext(const DGLDisplayState* dpy, GLContextVersion version,
              opaque_id_t id, const GLContextCreationData& creationData);
    ~GLContext();

    dglnet::message::utils::ContextReport describe();

    NativeSurfaceBase* getNativeReadSurface() const;
    NativeSurfaceBase* getNativeDrawSurface() const;
    void setNativeSurfaces(NativeSurfaceBase* read, NativeSurfaceBase* draw);

    std::shared_ptr<dglnet::DGLResource> queryTexture(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryBuffer(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryFramebuffer(gl_t bufferEnum);
    std::shared_ptr<dglnet::DGLResource> queryFBO(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryRenderbuffer(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryShader(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryProgram(gl_t name);
    std::shared_ptr<dglnet::DGLResource> queryGPU();
    std::shared_ptr<dglnet::DGLResource> queryState(gl_t name);

    /**
     * texture level query (dispatches to API-specific query implementation)
     */
    std::shared_ptr<dglnet::resource::DGLPixelRectangle> queryTextureLevel(
            const GLTextureObj* tex, int level, int layer, int face,
            state_setters::PixelStoreAlignment&);

    /**
     * texture level query (OpenGL, using getters)
     */
    std::shared_ptr<dglnet::resource::DGLPixelRectangle>
            queryTextureLevelGetters(
                    const GLTextureObj* tex, int level, int layer, int face,
                    state_setters::PixelStoreAlignment& defAlignment);

    /**
     * texture level query (OpenGL ES, using auxiliary context)
     */
    std::shared_ptr<dglnet::resource::DGLPixelRectangle>
            queryTextureLevelAuxCtx(const GLTextureObj* tex, int level, int layer, int face);

    bool isTexture1Dim(GLenum target);
    bool isTexture2Dim(GLenum target);

    /**
     * texture level size query (using getters, or bisection)
     */
    void queryTextureLevelSize(const GLTextureObj* tex, GLuint level,
                               GLint* width, GLint* height, GLint* depth);

    bool textureProbeSizeES(GLenum levelTarget, int level, GLenum formatToRequest, GLenum typeToRequest, const int sizes[3]);
    int textureBisectSizeES(GLenum levelTarget, int level, int coord, int maxSize, GLenum formatToRequest, GLenum typeToRequest);


    /**
     * buffer query using getters
     */
    std::shared_ptr<dglnet::DGLResource> queryBufferGetters(GLBufferObj* buff);

    /**
     * buffer query using auxaliary ctx.
     */
    std::shared_ptr<dglnet::DGLResource> queryBufferAuxCtx(GLBufferObj* buff);

    opaque_id_t getId() const;

    std::pair<bool, GLenum> getPokedError();
    GLenum peekError();

    /**
     * Debugger's specific debug message callback
     */
    static void KHRONOS_APIENTRY
            debugOutputCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar* message, const GLvoid* userParam);

    /**
     * non-static function called from debug message callback
     */
    void setDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
                        GLsizei length, const GLchar* message,
                        const GLvoid* userParam);

    /**
     * Returns true, if got debug message output
     */
    bool hasDebugOutput();

    /**
     * Returns last debug message
     */
    const std::string& popDebugOutput();

    /**
     * Setter for custom debug message callback, that can be registered by
     * application
     */
    void setCustomDebugOutputCallback(GLDEBUGPROC callback);

    void startQuery();
    bool endQuery(std::string& message);

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
    const GLContextVersion& getVersion() const;

    enum class ContextCap {
        PixelBufferObjects,
        FramebufferObjects,
        SeparateReadDrawFramebufferObjects,
        DrawBuffersMRT,
        TextureMultisample,
        RenderBufferMultisample,
        TextureQueryStencilBits,
        ReadBuffersFrontBuffer,
        ReadBuffer,
        CanQueryFramebufferAttachmentBitSize,
        MultipleFramebufferAttachments,
        Has64BitGetters,
        HasGetStringI,
        TextureGetters,
        GetBufferSubData,
        GLSLShaders,
        GenericVertexAttribs,
    };

    /**
     * Context capability check
     */
    bool hasCapability(ContextCap) const;

    /**
     * Getter for context creation data
     */
    const GLContextCreationData& getContextCreationData() const;

    /**
     * Auxiliary context getter
     */
    GLAuxContext* getAuxContext();

    /**
     *  Getter for parent display
     */
    const DGLDisplayState* getDisplay() const;

    /** 
     * Getter for shadow state;
     */
    inline GLContextShadowState& shadow() { return m_ShadowState; }

    /** 
     * Getter for objects shadow namespace
     */
    inline GLObjectNameSpaces& ns() { return m_ObjectNamespace; }

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
     * Handle to native surface (drawable), read
     */
    NativeSurfaceBase* m_NativeReadSurface;

    /**
     * Handle to native surface (drawable), draw
     */
    NativeSurfaceBase* m_NativeDrawSurface;

    /**
     * Queue for errors poked from glGetError(), not yet delivered to
     * application
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
     * Custom debug message callback registered by application
     */
    GLDEBUGPROC m_DebugOutputCallback;

    /**
     * Get state element (using glGetIntegerv)
     */
    void getStateIntegerv(
            const char* name, GLenum value, size_t length, 
            dglnet::resource::utils::StateItem* ret);

    /**
    * Get state element (using glGetInteger64v)
    */
    void getStateInteger64v(
            const char* name, GLenum value, size_t length, 
            dglnet::resource::utils::StateItem* ret);

    /**
     * Get state element (using glGetFloatv)
     */
    void getStateFloatv(
            const char* name, GLenum value, size_t length, 
            dglnet::resource::utils::StateItem* ret);

    /**
     * Get state element (using glGetDoublev)
     */
    void getStateDoublev(
            const char* name, GLenum value, size_t length,
            dglnet::resource::utils::StateItem* ret);

    /**
     * Get state element (using glGetBooleanv)
     */
    void getStateBooleanv(
            const char* name, GLenum value, size_t length,
            dglnet::resource::utils::StateItem* ret);

    /**
     * Get state element (using glIsEnabled)
     */
    void getStateIsEnabled(
            const char* name, GLenum value, size_t,
            dglnet::resource::utils::StateItem* ret);

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

    /**
     * True if queries are now performed on contexts
     */
    bool m_InQuery;

    /**
     * Context creation data - context attributes used on creation of this ctx.
     */
    GLContextCreationData m_CreationData;

    /**
     * Auxiliary context (used on demand, when query cannot be performed on this
     * ctx).
     */
    std::shared_ptr<GLAuxContext> m_AuxContext;

    /**
     * Parent display
     */
    const DGLDisplayState* m_Display;

    /** 
     * Shadowed state containter
     * 
     * All non-query-able state goes in here
     */
    GLContextShadowState m_ShadowState;

    /** 
     * Namespace of shadows for specific GL objects
     */
    GLObjectNameSpaces m_ObjectNamespace;

};

}    // namespace
#endif
