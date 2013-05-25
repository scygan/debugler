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


#include "dglblitterbase.h"

#include <DGLNet/protocol/pixeltransfer.h>

DGLBlitterBase::DGLBlitterBase() {
    for (size_t i = 0; i < sizeof(m_ChannelScaleBiases)/sizeof(m_ChannelScaleBiases[0]); i++) {
        m_ChannelScaleBiases[i] = std::pair<float, float>(1.0f, 0.0f);
    }
}

void DGLBlitterBase::blit(unsigned int width, unsigned int height, unsigned int rowBytes, GLenum format, GLenum type, const void* data) {

    m_SrcStride = rowBytes;
    m_Width = width;
    m_Height = height;
    m_DataFormat = GLFormats::getDataFormat(format);
    m_DataType = GLFormats::getDataType(type);

    if (!m_DataFormat || !m_DataType) {
        throw std::runtime_error("Got image of unknown type and format. It may be debugger bug");
    }

    m_SrcData = data;

    doBlit();
}

void  DGLBlitterBase::setChannelScale(Channel channel, float scale, float bias) {
    m_ChannelScaleBiases[channel] = std::pair<float, float>(scale, bias);
    if (m_DataFormat && m_DataType) {
        doBlit();
    }
}

void DGLBlitterBase::doBlit() {
    
    OutputFormat outputFormat = _GL_RGBX32;
    std::pair<float, float> channelSBs[4] = {
        std::pair<float, float>(1.0f, 0.0f),
        std::pair<float, float>(1.0f, 0.0f),
        std::pair<float, float>(1.0f, 0.0f),
        std::pair<float, float>(1.0f, 0.0f)
    };

    switch (m_DataFormat->format) {
        case GL_RED:
        case GL_RG:       
        case GL_RGB:
        case GL_RED_INTEGER:
        case GL_RG_INTEGER:
        case GL_RGB_INTEGER:
            outputFormat = _GL_RGBX32;
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_R];
            channelSBs[1] = m_ChannelScaleBiases[CHANNEL_G];
            channelSBs[2] = m_ChannelScaleBiases[CHANNEL_B];
            break;
        case GL_RGBA:
        case GL_RGBA_INTEGER:
            outputFormat = _GL_BGRA32;
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_B];
            channelSBs[1] = m_ChannelScaleBiases[CHANNEL_G];
            channelSBs[2] = m_ChannelScaleBiases[CHANNEL_R];
            channelSBs[3] = m_ChannelScaleBiases[CHANNEL_A];
            break;
        case GL_DEPTH_COMPONENT:
            outputFormat = _GL_MONO8;
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_D];
            break;
        case GL_STENCIL_INDEX:
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_S];
            outputFormat = _GL_MONO8;
            break;
        case GL_ALPHA:
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_A];
            outputFormat = _GL_MONO8;
            break;
        case GL_LUMINANCE:
            outputFormat = _GL_MONO8; break;
        case GL_DEPTH_STENCIL:
            outputFormat = _GL_RGBX32;
            channelSBs[0] = m_ChannelScaleBiases[CHANNEL_D];
            channelSBs[1] = m_ChannelScaleBiases[CHANNEL_S];
            break;
        case GL_LUMINANCE_ALPHA:
            outputFormat = _GL_RGBX32; break;
        default:
        assert(0);
    }
    
    int srcPixelSize = 0;
    if (m_DataType->packed) {
        srcPixelSize = m_DataType->byteSize;
    } else {
        srcPixelSize =  m_DataFormat->components * m_DataType->byteSize;
    }  

    int dstPixelSize = 0;
    for (int i = 0; i < 4; i++) {
        if (outputOffsets[outputFormat][i] >= 0)
            dstPixelSize ++;
    }

    int targetRowBytes = (m_Width * dstPixelSize + 4 - 1) & (-4);
    if (outputData.size() < size_t(targetRowBytes * m_Height))
        outputData = std::vector<char>(targetRowBytes * m_Height);

    m_DataType->blitFunc(outputOffsets[outputFormat], m_Width, m_Height, m_SrcData, &outputData[0], m_SrcStride, targetRowBytes, srcPixelSize, dstPixelSize, m_DataFormat->components, channelSBs);

    sink(m_Width, m_Height, outputFormat, &outputData[0]);
}

std::vector<AnyValue> DGLBlitterBase::describePixel(unsigned int x, unsigned int y) {

    if (!m_SrcStride || !m_DataType || !m_DataFormat || x > m_Width || y > m_Height) {
        return std::vector<AnyValue>();
    }

    int srcPixelSize = 0;
    if (m_DataType->packed) {
        srcPixelSize = m_DataType->byteSize;
    } else {
        srcPixelSize =  m_DataFormat->components * m_DataType->byteSize;
    }  

    const void* pixPtr = &reinterpret_cast<const unsigned char*>(m_SrcData)[y * m_SrcStride + x * srcPixelSize];

    return m_DataType->extractor(pixPtr, m_DataFormat->components);
}

const int DGLBlitterBase::outputOffsets[3][4] = {
    {2, 1, 0, 3},
    {0, 1, 2, -1},
    {0, -1, -1, -1}
};
