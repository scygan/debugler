#ifndef GL_FORMATS_H
#define GL_FORMATS_H

#include <DGLCommon/gl-headers.h>
#include <DGLCommon/gl-serialized.h>

#include <vector>

struct GLDataFormat;
struct GLDataType;

class DGLBlitterBase {

public:
    DGLBlitterBase();

    void blit(unsigned int width, unsigned int height, unsigned int rowBytes, GLenum format, GLenum type, const void* data);

    enum Channel {
        CHANNEL_R,
        CHANNEL_G,
        CHANNEL_B,
        CHANNEL_A,
        CHANNEL_D,
        CHANNEL_S,
        _LAST_CHANNEL
    };

    void setChannelScale(Channel channel, float scale, float bias);

    std::vector<AnyValue> describePixel(unsigned int x, unsigned int y);

    enum OutputFormat {
        _GL_BGRA32, //alpha, DX-order format
        _GL_RGBX32, //non-alpha, RGBX format
        _GL_MONO8,  //monochromatic, 8 bit format
        _LAST
    };

    static const int outputOffsets[3][4];
protected: 
    virtual void sink(int width, int height, OutputFormat format, void* data) = 0;

    void doBlit();

private:
    const void* m_SrcData;

    unsigned int m_SrcStride;
    GLDataFormat* m_DataFormat;
    GLDataType* m_DataType;
    unsigned int m_Width, m_Height;

    std::vector<char> outputData;

    std::pair<float, float> m_ChannelScaleBiases[_LAST_CHANNEL];
};

struct GLDataType {
    GLenum type;
    unsigned int byteSize;
    bool packed;
    void (*blitFunc) (DGLBlitterBase::OutputFormat outputFormat, int width, int height, const void * src, void* dst, int srcStride, int dstStride, int srcPixelSize, int dstPixelSize, int srcComponents, std::pair<float, float>* scale);
    std::vector<AnyValue> (*extractor)(const void*, int);
    unsigned int components;
}; 

struct GLDataFormat {
    GLenum format;
    unsigned int components;
};

struct GLInternalFormat {
    GLenum internalFormat;
    GLenum dataFormat;
    GLenum dataType;
};
   

class GLFormats {
public:
    static GLInternalFormat* getInternalFormat(GLenum internalFormat);
    static GLDataFormat* getDataFormat(GLenum dataFormat);
    static GLDataType* getDataType(GLenum dataType);  
};

class DGLPixelTransfer {
public: 
    DGLPixelTransfer(std::vector<GLint> _rgbaSizes, std::vector<GLint> _depthStencilSizes, GLenum internalFormat);

    bool isValid();
    GLenum getFormat();
    GLenum getType();
    unsigned int getPixelSize();

private:
    GLDataFormat* m_DataFormat;
    GLDataType* m_DataType;
};

#endif
