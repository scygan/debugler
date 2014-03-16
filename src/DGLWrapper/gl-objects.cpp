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

#include "gl-objects.h"
#include "pointers.h"

#include <DGLCommon/def.h> 

#include <cassert>

namespace dglState {

GLObj::GLObj() : m_Name(0) {}

GLObj::GLObj(GLuint name) : m_Name(name) {}

GLuint GLObj::getName() const { return m_Name; }

GLTextureObj::GLTextureObj(GLuint name) : GLObj(name), m_Target(0) {}

void GLTextureObj::setTarget(GLenum target) {
    if (!m_Target) m_Target = target;
}

void GLTextureObj::setTexImage(GLuint level, GLsizei width, GLsizei height,
                               GLsizei depth, GLenum internalFormat, GLenum,
                               GLenum type) {
    if (m_Levels.size() < static_cast<size_t>((level + 1))) {
        m_Levels.resize(level + 1);
    }

    m_Levels[level] =
            GLTextureLevel(internalFormat, type, width, height, depth);
}

void GLTextureObj::setTexStorage(GLuint levels, GLsizei width, GLsizei height,
                                 GLsizei depth, GLenum internalFormat,
                                 GLenum format, GLenum type) {
    int levelWidth = width;
    int levelHeight = height;
    int levelDepth = depth;

    m_Levels.resize(levels);

    for (GLuint i = 0; i < levels; i++) {
        setTexImage(i, levelWidth, levelHeight, levelDepth, internalFormat,
                    format, type);
        levelWidth = std::max(1, (levelWidth / 2));
        levelHeight = std::max(1, (levelHeight / 2));
        levelDepth = std::max(1, (levelDepth / 2));
    }
}

GLenum GLTextureObj::getTarget() const { return m_Target; }

const GLTextureObj::GLTextureLevel* GLTextureObj::getRequestedLevel(GLint level)
        const {
    if (static_cast<size_t>(level) < m_Levels.size()) {
        return &m_Levels[level];
    } else {
        return NULL;
    }
}

GLenum GLTextureObj::getTextureLevelTarget(int face) const {
    if (m_Target == GL_TEXTURE_CUBE_MAP) {
        GLenum cubeMapFaces[] = {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
        assert((size_t)face < sizeof(cubeMapFaces) / sizeof(cubeMapFaces[0]));
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

GLBufferObj::GLBufferObj(GLuint name) : GLObj(name), m_Target(0) {}

void GLBufferObj::setTarget(GLenum target) {
    if (!m_Target) m_Target = target;
}

GLenum GLBufferObj::getTarget() { return m_Target; }

GLProgramObj::GLProgramObj(GLuint name, bool arbApi)
        : GLObj(name), m_InUse(false), m_arbApi(arbApi) {}

GLProgramObj::~GLProgramObj() {
    auto i = m_AttachedShaders.begin();
    while (i != m_AttachedShaders.end()) {
        detachShader(*(i++));
    }
}

void GLProgramObj::use(bool) { m_InUse = true; }

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

GLShaderObj::GLShaderObj(GLuint name, bool arbApi)
        : GLObj(name),
          m_Deleted(false),
          m_DeleteCalled(false),
          m_Target(0),
          m_arbApi(arbApi),
          m_RefCount(0) {}

void GLShaderObj::deleteCalled() {
    m_DeleteCalled = true;
    mayDelete();
}

void GLShaderObj::incRefCount() { m_RefCount++; }
void GLShaderObj::decRefCount() {
    m_RefCount--;
    mayDelete();
}

int GLShaderObj::getRefCount() { return m_RefCount; }

GLint GLShaderObj::queryCompilationStatus() const {
    if (m_Deleted) {
        return 0;
    }
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
    if (m_Deleted) {
        return "";
    }

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

const std::string& GLShaderObj::querySource() {
    if (isDeleted()) {
        // shader does not exist in shader memory any more
        return m_Source;
    }

    std::vector<GLchar> sources;
    GLint length;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetObjectParameterivARB)(
                getName(), GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, &length);
    } else {
        DIRECT_CALL_CHK(glGetShaderiv)(getName(), GL_SHADER_SOURCE_LENGTH,
                                       &length);
    }
    sources.resize(length + 1);
    GLint actualLength;
    if (m_arbApi) {
        DIRECT_CALL_CHK(glGetShaderSourceARB)(getName(), length, &actualLength,
                                              &sources[0]);
    } else {
        DIRECT_CALL_CHK(glGetShaderSource)(getName(), length, &actualLength,
                                           &sources[0]);
    }
    sources[std::min(sources.size() - 1, static_cast<size_t>(actualLength))] =
            0;

    m_Source = &sources[0];

    return m_Source;
}

void GLShaderObj::shaderSourceCalled() {
    if (isDeleted()) {
        throw std::runtime_error(
                "glShaderSource was called, but shader already marked as "
                "deleted");
    }
    m_OrigSource = querySource();
}

bool GLShaderObj::isDeleted() const { return m_Deleted; }

void GLShaderObj::editSource(const std::string& source) {
    if (isDeleted()) {
        throw std::runtime_error(
                "Error: trying to edit source of already deleted shader");
    }

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

void GLShaderObj::mayDelete() {
    if (m_RefCount == 0 && m_DeleteCalled) {
        m_Deleted = true;
    }
}

GLenum GLShaderObj::getTarget() const { return m_Target; }

void GLShaderObj::createCalled(GLenum target) {
    m_Target = target;
    m_Deleted = false;
    m_DeleteCalled = false;
    m_RefCount = 0;
}

GLFBObj::GLFBObj(GLuint name) : GLObj(name), m_Target(0) {}

void GLFBObj::setTarget(GLenum target) {
    if (!m_Target) m_Target = target;
}

GLenum GLFBObj::getTarget() { return m_Target; }

}    // namespace dglState
