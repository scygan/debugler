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

#include "gl-context.h"

#include "gl-objects.h"
#include "pointers.h"

#include <DGLCommon/def.h> 

#include <sstream>

namespace dglState {

GLObj::GLObj() : m_Name(0), m_Target(0) {}

GLObj::GLObj(GLuint name) : m_Name(name), m_Target(0) {}

GLuint GLObj::getName() const { return m_Name; }

void GLObj::setTarget(GLenum target) {
    if (!m_Target) m_Target = target;
}

GLenum GLObj::getTarget() const { return m_Target; }

GLTextureObj::GLTextureObj(GLuint name) : GLObj(name) {}

void GLTextureObj::setTexImage(GLuint level, GLsizei width, GLsizei height,
                               GLsizei depth, GLenum internalFormat, GLenum,
                               GLenum type) {
    if (m_Levels.size() < static_cast<size_t>((static_cast<size_t>(level) + 1))) {
        m_Levels.resize(static_cast<size_t>(level) + 1);
    }

    m_Levels[static_cast<size_t>(level)] =
            GLTextureLevel(internalFormat, type, width, height, depth);
}

void GLTextureObj::setTexStorage(GLuint levels, GLsizei width, GLsizei height,
                                 GLsizei depth, GLenum internalFormat,
                                 GLenum format, GLenum type) {
    int levelWidth = width;
    int levelHeight = height;
    int levelDepth = depth;

    m_Levels.resize(static_cast<size_t>(levels));

    for (GLuint i = 0; i < levels; i++) {
        setTexImage(i, levelWidth, levelHeight, levelDepth, internalFormat,
                    format, type);
        levelWidth = std::max(1, (levelWidth / 2));
        levelHeight = std::max(1, (levelHeight / 2));
        levelDepth = std::max(1, (levelDepth / 2));
    }
}

void GLTextureObj::getFormat(GLContext* ctx, int level, GLenum levelTarget, GLint& retInternalFormat, GLint& retSamples) const {

    retInternalFormat = 0; 
    retSamples = 0;

    if (ctx->hasCapability(GLContext::ContextCap::TextureMultisample)) {
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(levelTarget, level,
            GL_TEXTURE_SAMPLES, &retSamples);
    }

    if (ctx->hasCapability(GLContext::ContextCap::TextureGetters)) {
        DIRECT_CALL_CHK(glGetTexLevelParameteriv)(
            levelTarget, level, GL_TEXTURE_INTERNAL_FORMAT,
            &retInternalFormat);
    } else {
        if (m_Levels.size()) {
            retInternalFormat = m_Levels[0].m_RequestedInternalFormat;
        }
    }
}

const GLTextureObj::GLTextureLevel* GLTextureObj::getRequestedLevel(GLint level)
        const {
    if (static_cast<size_t>(level) < m_Levels.size()) {
        return &m_Levels[static_cast<size_t>(level)];
    } else {
        return NULL;
    }
}

GLenum GLTextureObj::getTextureLevelTarget(size_t face) const {
    if (getTarget() == GL_TEXTURE_CUBE_MAP) {
        GLenum cubeMapFaces[] = {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
        DGL_ASSERT((size_t)face < DGL_ARRAY_LENGTH(cubeMapFaces));
        return cubeMapFaces[face];
    } else {
        return getTarget();
    }
}

GLTextureObj::GLTextureLevel::GLTextureLevel()
        : m_RequestedInternalFormat(0),
          m_RequestedDataType(0),
          m_Width(0),
          m_Height(0),
          m_Depth(0) {}

GLTextureObj::GLTextureLevel::GLTextureLevel(GLenum requestedInternalFormat,
                                             GLenum requestedDataType,
                                             GLsizei width, GLsizei height,
                                             GLsizei depth)
        : m_RequestedInternalFormat(requestedInternalFormat),
          m_RequestedDataType(requestedDataType),
          m_Width(width),
          m_Height(height),
          m_Depth(depth) {}

GLBufferObj::GLBufferObj(GLuint name) : GLObj(name) {}

GLProgramObj::GLProgramObj(GLuint name, bool arbApi)
        : GLObj(name), m_InUse(false), m_Deleted(false), m_arbApi(arbApi) {}

GLProgramObj::~GLProgramObj() {
    auto i = m_AttachedShaders.begin();
    while (i != m_AttachedShaders.end()) {
        detachShader(*(i++));
    }
}

void GLProgramObj::setInUse(bool inUse) { m_InUse = inUse; }

bool GLProgramObj::mayDelete() { return !m_InUse && m_Deleted; }

void GLProgramObj::markDeleted() { m_Deleted = true; }

void GLProgramObj::attachShader(GLShaderObj* shader) {
    m_AttachedShaders.insert(shader);
    shader->incRefCount();
}

void GLProgramObj::detachShader(GLShaderObj* shader) {
    m_AttachedShaders.erase(shader);
    shader->decRefCount();
}

std::set<GLShaderObj*>& GLProgramObj::getAttachedShaders() {
    return m_AttachedShaders;
}

void GLProgramObj::forceLink() {
    if (m_arbApi) {
        DIRECT_CALL_CHK(glLinkProgramARB)(getName());
    } else {
        DIRECT_CALL_CHK(glLinkProgram)(getName());
    }

    GLint currentProgram = 0;
    DIRECT_CALL_CHK(glGetIntegerv)(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram == (GLint)getName()) {
        // reinstall program
        DIRECT_CALL_CHK(glUseProgram)(getName());
    }
}

void GLProgramObj::setEmbeddedSSOSource(GLsizei count, const char* const* strings) {
    
    DGL_ASSERT(m_AttachedShaders.size() == 0);

    std::ostringstream sourceStr; 

    for (size_t i = 0; i < static_cast<size_t>(count); i++)  {
        sourceStr << strings[i]; 
    }

    m_EmbeddedSSOSource = sourceStr.str();

}

GLShaderObj::GLShaderObj(GLuint name, GLShaderObjCreateData createData)
        : GLObj(name),
          m_DeleteCalled(false),
          m_arbApi(createData.m_ArbApi),
          m_RefCount(0), 
          m_Parrent(createData.m_Parrent) {}

void GLShaderObj::deleteCalled() {
    m_DeleteCalled = true;
    deleteSelfIfNeeded();
}

void GLShaderObj::incRefCount() { m_RefCount++; }
void GLShaderObj::decRefCount() {
    m_RefCount--;
    deleteSelfIfNeeded();
}

int GLShaderObj::getRefCount() { return m_RefCount; }

GLint GLShaderObj::queryCompilationStatus() const {
    GLint compileStatus;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(
                getName(), GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_COMPILE_STATUS,
                                       &compileStatus);
    }
    return compileStatus;
}

std::string GLShaderObj::queryCompilationInfoLog() const {
    GLint infoLogLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(
                getName(), GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_INFO_LOG_LENGTH,
                                       &infoLogLength);
    }

    std::vector<GLchar> infoLog(infoLogLength + 1);

    GLint actualLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetInfoLogARB)(getName(),
                                         static_cast<GLsizei>(infoLog.size()),
                                         &actualLength, &infoLog[0]);
    } else {
        DIRECT_CALL_CHK(glGetShaderInfoLog)(
                getName(), static_cast<GLsizei>(infoLog.size()), &actualLength,
                &infoLog[0]);
    }
    infoLog[std::min(infoLog.size() - 1, static_cast<size_t>(actualLength))] =
            0;

    return &infoLog[0];
}

std::string GLShaderObj::querySource() {
    std::vector<GLchar> sources;
    GLint length = 0;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(
                getName(), GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &length);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_SHADER_SOURCE_LENGTH,
                                       &length);
    }

    sources.resize(static_cast<size_t>(length) + 1);

    GLint actualLength = 0;

    if (length) {
        
        if (m_arbApi) {
            DIRECT_CALL_CHK(glGetShaderSourceARB)(getName(), length, &actualLength,
                &sources[0]);
        } else {
            DIRECT_CALL_CHK(glGetShaderSource)(getName(), length, &actualLength,
                &sources[0]);
        }
    }

    sources[std::min(sources.size() - 1, static_cast<size_t>(actualLength))] =
        0;

    return &sources[0];
}

void GLShaderObj::shaderSourceCalled() {
    m_OrigSource = querySource();
}

void GLShaderObj::editSource(const std::string& source) {
    const char* sourcePtr = source.c_str();
    if (m_arbApi) {
        DIRECT_CALL_CHK(glShaderSourceARB)(getName(), 1, &sourcePtr, NULL);
        DIRECT_CALL_CHK(glCompileShaderARB)(getName());
    } else {
        DIRECT_CALL_CHK(glShaderSource)(getName(), 1, &sourcePtr, NULL);
        DIRECT_CALL_CHK(glCompileShader)(getName());
    }
}

void GLShaderObj::resetSourceToOrig() { editSource(m_OrigSource); }

void GLShaderObj::deleteSelfIfNeeded() {
    if (m_RefCount == 0 && m_DeleteCalled) {
        m_Parrent->m_Shaders.deleteObject(getName());
    }
}

void GLShaderObj::createCalled(GLenum target) {
    setTarget(target);
    m_DeleteCalled = false;
    m_RefCount = 0;
}


std::set<dglnet::ContextObjectName> GLProgramPipelineObj::getReport(opaque_id_t ctxId) {
    std::set<dglnet::ContextObjectName> ret; 

    for (std::map<GLbitfield, GLuint>::iterator it = m_Programs.begin(); 
        it != m_Programs.end(); 
        ++it) {


        ret.insert(dglnet::ContextObjectName(ctxId, it->second, it->first));

    }

    return ret;
}

void GLProgramPipelineObj::useProgramStages(GLbitfield /*stages*/, GLuint /*program*/) {
   
    //TODO: need to rethink this part..
    
    /*
    GLuint usedStages = stages;

    GLProgramObj* programObj = nullptr; 


    if (program != 0) {
        programObj = m_Parrent.m_Programs.getOrCreateObject(program);
        usedStages &= programObj->getLinkedInShadersMask();
    }

    for (GLbitfield i = 1; i < (1 << (sizeof(GLbitfield) * 8 - 1)); i << 1) {
        GLuint stage = (usedStages & i);

        if (programObj) {

            //if it's new refcount it

        } else {

            //if it's not longer used unrefcount it.
        }
    }
    */
}

GLFBObj::GLFBObj(GLuint name) : GLObj(name) {}

GLRenderbufferObj::GLRenderbufferObj(GLuint name) : GLObj(name) {}

}    // namespace dglState
