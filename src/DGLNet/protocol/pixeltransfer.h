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

#ifndef PIXELTRANSFER_H
#define PIXELTRANSFER_H

#include <DGLCommon/gl-types.h>
#include <DGLNet/protocol/anyvalue.h>

#include <vector>

struct GLDataType {
    gl_t type;
    unsigned int byteSize;
    bool packed;
    void (*blitFunc)(const int* outputOffsets, size_t width, size_t height,
                     const void* src, void* dst, size_t srcStride, size_t dstStride,
                     size_t srcPixelSize, size_t dstPixelSize, int srcComponents,
                     std::pair<float, float>* scale);
    std::vector<AnyValue>(*extractor)(const void*, int);
    unsigned int components;
};

struct GLDataFormat {
    gl_t format;
    unsigned int components;
};

#define RENDERABLE_ES2 (1 << 0)
#define RENDERABLE_ES3 (1 << 1)

struct GLInternalFormat {
    gl_t internalFormat;
    gl_t dataFormat;
    gl_t dataType;
    unsigned int colorRenderable;
};

class GLFormats {
   public:
    static const GLInternalFormat* getInternalFormat(gl_t internalFormat);

    static const GLDataFormat* getDataFormat(gl_t dataFormat);
    static const GLDataType* getDataType(gl_t dataType);

    static gl_t getBestColorRenderableFormatES(gl_t internalFormat, gl_t type,
                                          int ctxMajor);
    static const GLInternalFormat* adjustInternalFormatFromTypeES(
            gl_t internalFormat, gl_t type);
};

class DGLPixelTransfer {
   public:
    DGLPixelTransfer();

    // OpenGL preferred initializer
    bool initializeOGL(GLenum internalFormat,
                       std::vector<GLint> _rgbaSizes = std::vector<GLint>(4, 0),
                       std::vector<GLint> _depthStencilSizes =
                               std::vector<GLint>(2, 0));

    // OpenGL ES preferred initializer
    bool initializeOGLES(GLenum internalFormat, GLenum implReadFormat,
                         GLenum implReadType);

    bool isValid();
    gl_t getFormat();
    gl_t getType();
    unsigned int getPixelSize();

   private:
    const GLDataFormat* m_DataFormat;
    const GLDataType* m_DataType;
};

#endif
