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


#include "resource.h"
#include <cstring>



namespace dglnet {
namespace resource {

DGLResourceFBO::FBOAttachment::FBOAttachment(gl_t id) : m_Id(id), m_Ok(true) {}

void DGLResourceFBO::FBOAttachment::error(const std::string& msg) {
    m_Ok = false;
    m_ErrorMsg = msg;
}

bool DGLResourceFBO::FBOAttachment::isOk(std::string& msg) const {
    msg = m_ErrorMsg;
    return m_Ok;
}

DGLPixelRectangle::DGLPixelRectangle(value_t width, value_t height,
                                     value_t rowBytes, gl_t glFormat,
                                     gl_t glType)
        : m_RowBytes(rowBytes),
          m_GLFormat(glFormat),
          m_GLType(glType),
          m_Storage(NULL),
          m_Width(width),
          m_Height(height) {

    if (m_Height * m_RowBytes) {
        m_Storage = malloc(getSize());
    }
}

DGLPixelRectangle::DGLPixelRectangle(const DGLPixelRectangle& rhs)
        : m_GLFormat(rhs.m_GLFormat),
          m_GLType(rhs.m_GLType),
          m_Width(rhs.m_Width),
          m_Height(rhs.m_Height),
          m_RowBytes(rhs.m_RowBytes) {
    if (rhs.getPtr()) {
        m_Storage = malloc(getSize());
        memcpy(m_Storage, rhs.getPtr(), getSize());
    } else {
        m_Storage = NULL;
    }
}

DGLPixelRectangle& DGLPixelRectangle::operator= (const DGLPixelRectangle& rhs) {
    m_Width = rhs.m_Width;
    m_Height = rhs.m_Height;
    m_RowBytes = rhs.m_RowBytes;
    m_GLFormat = rhs.m_GLFormat;
    m_GLType = rhs.m_GLType;
    if (rhs.getPtr()) {
        m_Storage = malloc(getSize());
        memcpy(m_Storage, rhs.getPtr(), getSize());
    } else {
        m_Storage = NULL;
    }
    return *this;
}

DGLPixelRectangle::~DGLPixelRectangle() {
    if (m_Storage) {
        free(m_Storage);
        m_Storage = NULL;
    }
}

void* DGLPixelRectangle::getPtr() const { return m_Storage; }

size_t DGLPixelRectangle::getSize() const { return static_cast<size_t>(m_Height * m_RowBytes); }

}    // namespace resource
}    // namespace dglnet
