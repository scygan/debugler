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


#include "pixeltransfer.h"

#include <stdexcept>
#include <cassert>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

namespace blt {

    inline void blitUNORM8(const void* inVoid, int components, float* out) {
        const uint8_t* inCast = reinterpret_cast<const uint8_t*>(inVoid);
        out[0] = inCast[0] / 255.f;
        if (components > 1)    
            out[1] = inCast[1] / 255.f;
        if (components > 2)    
            out[2] = inCast[2] / 255.f;
        if (components > 3)    
            out[3] = inCast[3] / 255.f;
    }
    inline void blitSNORM8(const void* inVoid, int components, float* out) {
        const int8_t* inCast = reinterpret_cast<const int8_t*>(inVoid);
        out[0] = MAX(inCast[0] / 127.f, -1.0f);
        if (components > 1)    
            out[1] = MAX(inCast[1] / 127.f, -1.0f);
        if (components > 2)    
            out[2] = MAX(inCast[2] / 127.f, -1.0f);
        if (components > 3)    
            out[3] = MAX(inCast[3] / 127.f, -1.0f);
    }

    inline void blitUNORM16(const void* inVoid, int components, float* out) {
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        out[0] = inCast[0] / 65535.f;
        if (components > 1)    
            out[1] = inCast[1] / 65535.f;
        if (components > 2)    
            out[2] = inCast[2] / 65535.f;
        if (components > 3)    
            out[3] = inCast[3] / 65535.f;
    }
    inline void blitSNORM16(const void* inVoid, int components, float* out) {
        const int16_t* inCast = reinterpret_cast<const int16_t*>(inVoid);
        out[0] = MAX(inCast[0] / 32767.f, -1.0f);
        if (components > 1)    
            out[1] = MAX(inCast[1] / 32767.f, -1.0f);
        if (components > 2)    
            out[2] = MAX(inCast[2] / 32767.f, -1.0f);
        if (components > 3)    
            out[3] = MAX(inCast[3] / 32767.f, -1.0f);
    }

    inline void blitUNORM32(const void* inVoid, int components, float* out) {
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        out[0] = inCast[0] / 4294967295.f;
        if (components > 1)    
            out[1] = inCast[1] / 4294967295.f;
        if (components > 2)    
            out[2] = inCast[2] / 4294967295.f;
        if (components > 3)    
            out[3] = inCast[3] / 4294967295.f;
    }
    inline void blitSNORM32(const void* inVoid, int components, float* out) {
        const int32_t* inCast = reinterpret_cast<const int32_t*>(inVoid);
        out[0] = MAX(inCast[0] / 2147483647.f, -1.0f);
        if (components > 1)    
            out[1] = MAX(inCast[1] / 2147483647.f, -1.0f);
        if (components > 2)    
            out[2] = MAX(inCast[2] / 2147483647.f, -1.0f);
        if (components > 3)    
            out[3] = MAX(inCast[3] / 2147483647.f, -1.0f);
    }
    inline void blitFLOAT32(const void* inVoid, int components, float* out) {
        const float* inCast = reinterpret_cast<const float*>(inVoid);
        out[0] = inCast[0];
        if (components > 1)    
            out[1] = inCast[1];
        if (components > 2)    
            out[2] = inCast[2];
        if (components > 3)    
            out[3] = inCast[3];
    }

    inline void blitUNORM4444(const void* inVoid, int components, float* out) {
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        out[3] = (inCast[0]  & 0x0f) / float(0x0f);
        out[2] = ((inCast[0]  & (0x0f << 4)) >> 4) / float(0x0f);
        out[1] = ((inCast[0]  & (0x0f << 8)) >> 8) / float(0x0f);
        out[0] = ((inCast[0]  & (0x0f << 12)) >> 12) / float(0x0f);
    }

    inline void blitUNORM5551(const void* inVoid, int components, float* out) {
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        out[3] = static_cast<float>(inCast[0]  & 0x01);
        out[2] = ((inCast[0]  & (0x1f << 1)) >> 1) / float(0x1f);
        out[1] = ((inCast[0]  & (0x1f << 6)) >> 6) / float(0x1f);
        out[0] = ((inCast[0]  & (0x1f << 11)) >> 11) / float(0x1f);
    }

    inline void blitUNORM2101010_REV(const void* inVoid, int components, float* out) {
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        out[0] = (inCast[0]  & 0x3ff) / float(0x3ff);
        out[1] = ((inCast[0]  & (0x3ff << 10)) >> 10) / float(0x3ff);
        out[2] = ((inCast[0]  & (0x3ff << 20)) >> 20) / float(0x3ff);
        out[3] = ((inCast[0]  & (0x3   << 30)) >> 30) / float(0x3);
    }

    inline void blitUNORM565(const void* inVoid, int components, float* out) {
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        out[2] = (inCast[0]  & 0x1f) / float(0x1f);
        out[1] = ((inCast[0]  & (0x3f << 5)) >> 5) / float(0x3f);
        out[0] = ((inCast[0]  & (0x1f << 11)) >> 11) / float(0x1f);
    }

    inline void blitUNORM24_8(const void* inVoid, int components, float* out) {
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        out[1] = (inCast[0]  & 0xff) / float(0xff);
        out[0] = ((inCast[0]  & (0xffffff << 8)) >> 8) / float(0xffffff);
    }

    inline void blitF32_UNORM24_8(const void* inVoid, int components, float* out) {
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        out[1] = (inCast[1]  & 0xff) / float(0xff);
        out[0] = *reinterpret_cast<const float*>(&inCast[0]);
    }

    inline void blitUNORM332(const void* inVoid, int components, float* out) {
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        out[2] = (inCast[0]  & 0x3) / float(0x3);
        out[1] = ((inCast[0]  & (0x7 << 2)) >> 2) / float(0x7);
        out[0] = ((inCast[0]  & (0x7 << 5)) >> 5) / float(0x7);
    }

    template<void(*blitConversion)(const void*, int, float*)>
    static void blitFunc(const int* outputOffsets, int width, int height, const void * src, void* dst, int srcStride, int dstStride, int srcPixelSize, int dstPixelSize, int srcComponents, std::pair<float, float>* scaleBias) {
        for (int y = 0; y < height; y++) {            
            const unsigned char* srcPtr = (const unsigned char*)src + y * srcStride;
            unsigned char* dstPtr = (unsigned char*)dst + y * dstStride;
            for (int x = 0; x < width; x++) {         
                float temp[4] = { 0., 0., 0., 1.};

                blitConversion(srcPtr, srcComponents, temp);

                int out;

                if ((out = outputOffsets[0]) >= 0)
                    dstPtr[out] = static_cast<unsigned char>(MAX(MIN(temp[0] * scaleBias[out].first + scaleBias[out].second, 1.0f), 0.0f) * 255.0f);

                if ((out = outputOffsets[1]) >= 0)
                    dstPtr[out] = static_cast<unsigned char>(MAX(MIN(temp[1] * scaleBias[out].first + scaleBias[out].second, 1.0f), 0.0f) * 255.0f);

                if ((out = outputOffsets[2]) >= 0)
                    dstPtr[out] = static_cast<unsigned char>(MAX(MIN(temp[2] * scaleBias[out].first + scaleBias[out].second, 1.0f), 0.0f) * 255.0f);

                if ((out = outputOffsets[3]) >= 0)
                    dstPtr[out] = static_cast<unsigned char>(MAX(MIN(temp[3] * scaleBias[out].first + scaleBias[out].second, 1.0f), 0.0f) * 255.0f);

                dstPtr += dstPixelSize;               
                srcPtr += srcPixelSize;               
            }                                         
        }                                             
    }      
} //namespace blt 

namespace extract {

    template<typename T>
    std::vector<AnyValue> extract(const void* inVoid, int channels) {
        const T* inCast = reinterpret_cast<const T*>(inVoid);
        std::vector<AnyValue> ret;
        for (int i = 0; i < channels; i++) {
            ret.push_back(inCast[i]);
        }
        return ret;
    }

    std::vector<AnyValue> extractUNORM4444(const void* inVoid, int components) {
        std::vector<AnyValue> ret(4);
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        ret[3] = inCast[0]  & 0x0f;
        ret[2] = (inCast[0]  & (0x0f << 4)) >> 4;
        ret[1] = (inCast[0]  & (0x0f << 8)) >> 8;
        ret[0] = (inCast[0]  & (0x0f << 12)) >> 12;
        return ret;
    }

    std::vector<AnyValue> extractUNORM5551(const void* inVoid, int components) {
        std::vector<AnyValue> ret(4);
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        ret[3] = static_cast<float>(inCast[0]  & 0x01);
        ret[2] = (inCast[0]  & (0x1f << 1)) >> 1;
        ret[1] = (inCast[0]  & (0x1f << 6)) >> 6;
        ret[0] = (inCast[0]  & (0x1f << 11)) >> 11;
        return ret;
    }

    std::vector<AnyValue> extractUNORM2101010_REV(const void* inVoid, int components) {
        std::vector<AnyValue> ret(4);
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        ret[0] = inCast[0]  & 0x3ff;
        ret[1] = (inCast[0]  & (0x3ff << 10)) >> 10;
        ret[2] = (inCast[0]  & (0x3ff << 20)) >> 20;
        ret[3] = (inCast[0]  & (0x3   << 30)) >> 30;
        return ret;
    }

    std::vector<AnyValue> extractUNORM565(const void* inVoid, int components) {
        std::vector<AnyValue> ret(3);
        const uint16_t* inCast = reinterpret_cast<const uint16_t*>(inVoid);
        ret[2] = inCast[0]  & 0x1f;
        ret[1] = (inCast[0]  & (0x3f << 5)) >> 5;
        ret[0] = (inCast[0]  & (0x1f << 11)) >> 11;
        return ret;
    }

    std::vector<AnyValue> extractUNORM24_8(const void* inVoid, int components) {
        std::vector<AnyValue> ret(2);
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        ret[1] = inCast[0]  & 0xff;
        ret[0] = (inCast[0]  & (0xffffff << 8)) >> 8;
        return ret;
    }

    std::vector<AnyValue> extractF32_UNORM24_8(const void* inVoid, int components) {
        std::vector<AnyValue> ret(2);
        const uint32_t* inCast = reinterpret_cast<const uint32_t*>(inVoid);
        ret[1] = inCast[1]  & 0xff;
        ret[0] = *reinterpret_cast<const float*>(&inCast[0]);
        return ret;
    }

    std::vector<AnyValue> extractUNORM332(const void* inVoid, int components) {
        std::vector<AnyValue> ret(3);
        const uint8_t* inCast = reinterpret_cast<const uint8_t*>(inVoid);
        ret[2] = inCast[0]  & 0x3;
        ret[1] = (inCast[0]  & (0x7 << 2)) >> 2;
        ret[0] = (inCast[0]  & (0x7 << 5)) >> 5;
        return ret;
    }

} //namespace extract


GLDataType g_DataTypes[] = {
    { GL_UNSIGNED_BYTE,                    1, false, blt::blitFunc<blt::blitUNORM8>,           extract::extract<uint8_t>           },
    { GL_BYTE,                             1, false, blt::blitFunc<blt::blitSNORM8>,           extract::extract<int8_t>            },
    { GL_UNSIGNED_SHORT,                   2, false, blt::blitFunc<blt::blitUNORM16>,          extract::extract<uint16_t>          },
    { GL_SHORT,                            2, false, blt::blitFunc<blt::blitSNORM16>,          extract::extract<int16_t>           },
    { GL_UNSIGNED_INT,                     4, false, blt::blitFunc<blt::blitUNORM32>,          extract::extract<uint32_t>          },
    { GL_INT,                              4, false, blt::blitFunc<blt::blitSNORM32>,          extract::extract<int32_t>           },
    { GL_FLOAT,                            4, false, blt::blitFunc<blt::blitFLOAT32>,          extract::extract<float>             },
    { GL_UNSIGNED_SHORT_4_4_4_4,           2, true,  blt::blitFunc<blt::blitUNORM4444>,        extract::extractUNORM4444,        4 },
    { GL_UNSIGNED_SHORT_5_5_5_1,           2, true,  blt::blitFunc<blt::blitUNORM5551>,        extract::extractUNORM5551,        4 },
    { GL_UNSIGNED_INT_2_10_10_10_REV,      4, true,  blt::blitFunc<blt::blitUNORM2101010_REV>, extract::extractUNORM2101010_REV, 4 },
    { GL_UNSIGNED_SHORT_5_6_5,             2, true,  blt::blitFunc<blt::blitUNORM565>,         extract::extractUNORM565,         3 },
    { GL_UNSIGNED_INT_24_8,                4, true,  blt::blitFunc<blt::blitUNORM24_8>,        extract::extractUNORM24_8,        2 },
    { GL_FLOAT_32_UNSIGNED_INT_24_8_REV,   8, true,  blt::blitFunc<blt::blitF32_UNORM24_8>,    extract::extractF32_UNORM24_8,    2 },
    { GL_UNSIGNED_BYTE_3_3_2,              1, true,  blt::blitFunc<blt::blitUNORM332>,         extract::extractUNORM332,         2 },
};

GLDataFormat g_DataFormats[] = {
    { GL_RED,             1  },
    { GL_RG,              2  },
    { GL_RGB,             3  },
    { GL_RGBA,            4  },
    { GL_RED_INTEGER,     1  },
    { GL_RG_INTEGER,      2  },
    { GL_RGB_INTEGER,     3  },
    { GL_RGBA_INTEGER,    4  },
    { GL_DEPTH_COMPONENT, 1  },
    { GL_STENCIL_INDEX,   1  },
    { GL_DEPTH_STENCIL,   2  },
    { GL_ALPHA,           1  },
    { GL_LUMINANCE,       1  },
    { GL_LUMINANCE_ALPHA, 2  },
};

struct GLInternalFormat g_InternalFormats[] = {
    //available on ES3.0
    { GL_RGBA8,          GL_RGBA,         GL_UNSIGNED_BYTE                         },
    { GL_RGB5_A1,        GL_RGBA,         GL_UNSIGNED_BYTE                         },
    { GL_RGBA4,          GL_RGBA,         GL_UNSIGNED_BYTE                         },
    { GL_SRGB8_ALPHA8,   GL_RGBA,         GL_UNSIGNED_BYTE                         },
    { GL_RGBA8_SNORM,    GL_RGBA,         GL_BYTE                                  },
    { GL_RGBA4,          GL_RGBA,         GL_UNSIGNED_SHORT_4_4_4_4                },
    { GL_RGB5_A1,        GL_RGBA,         GL_UNSIGNED_SHORT_5_5_5_1                },
    { GL_RGB10_A2,       GL_RGBA,         GL_UNSIGNED_INT_2_10_10_10_REV           },
    { GL_RGB5_A1,        GL_RGBA,         GL_UNSIGNED_INT_2_10_10_10_REV           },
    //we get this one of GL_FLOAT                                                    
    //{ GL_RGBA16F,        GL_RGBA,         GL_HALF_FLOAT                          },
    { GL_RGBA16F,        GL_RGBA,         GL_FLOAT                                 },
    { GL_RGBA32F,        GL_RGBA,         GL_FLOAT                                 },
    { GL_RGBA8UI,        GL_RGBA_INTEGER, GL_UNSIGNED_BYTE                         },
    { GL_RGBA8I,         GL_RGBA_INTEGER, GL_BYTE                                  },
    { GL_RGBA16UI,       GL_RGBA_INTEGER, GL_UNSIGNED_SHORT                        },
    { GL_RGBA16I,        GL_RGBA_INTEGER, GL_SHORT                                 },
    { GL_RGBA32UI,       GL_RGBA_INTEGER, GL_UNSIGNED_INT                          },
    { GL_RGBA32I,        GL_RGBA_INTEGER, GL_INT                                   },
    { GL_RGB10_A2UI,     GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV           },
    //we get this one as packed 5_6_5, so no conversion occurs                       
    //{ GL_RGB565,         GL_RGB,          GL_UNSIGNED_BYTE                       },
    { GL_RGB8,           GL_RGB,          GL_UNSIGNED_BYTE                         },
    { GL_SRGB8,          GL_RGB,          GL_UNSIGNED_BYTE                         },
    { GL_RGB8_SNORM,     GL_RGB,          GL_BYTE                                  },
    { GL_RGB565,         GL_RGB,          GL_UNSIGNED_SHORT_5_6_5                  },
    //following 2 are too cumbersome (we get them as GL_FLOAT)                       
    //{ GL_R11F_G11F_B10F, GL_RGB,          GL_UNSIGNED_INT_10F_11F_11F_REV        },
    //{ GL_RGB9_E5,        GL_RGB,          GL_UNSIGNED_INT_5_9_9_9_REV            },
    //we get following three as GL_FLOAT                                           
    //{ GL_RGB16F,         GL_RGB,          GL_HALF_FLOAT                          },
    //{ GL_R11F_G11F_B10F, GL_RGB,          GL_HALF_FLOAT                          },
    //{ GL_RGB9_E5,        GL_RGB,          GL_HALF_FLOAT                          },
    { GL_RGB16F,         GL_RGB,          GL_FLOAT                                 },
    { GL_RGB32F,         GL_RGB,          GL_FLOAT                                 },
    { GL_R11F_G11F_B10F, GL_RGB,          GL_FLOAT                                 },
    { GL_RGB9_E5,        GL_RGB,          GL_FLOAT                                 },
    { GL_RGB8UI,         GL_RGB_INTEGER,  GL_UNSIGNED_BYTE                         },
    { GL_RGB8I,          GL_RGB_INTEGER,  GL_BYTE                                  },
    { GL_RGB16UI,        GL_RGB_INTEGER,  GL_UNSIGNED_SHORT                        },
    { GL_RGB16I,         GL_RGB_INTEGER,  GL_SHORT                                 },

    { GL_RGB32UI,        GL_RGB_INTEGER, GL_UNSIGNED_INT                           },       
    { GL_RGB32I,         GL_RGB_INTEGER, GL_INT                                    },        
    { GL_RG8,            GL_RG,          GL_UNSIGNED_BYTE                          },                    
    { GL_RG8_SNORM,      GL_RG,          GL_BYTE                                   },
    //We use GL_FLOATS for this one                                               
    //{ GL_RG16F,          GL_RG,          GL_HALF_FLOAT                           },                  
    { GL_RG32F,          GL_RG,          GL_FLOAT                                  },                  
    { GL_RG16F,          GL_RG,          GL_FLOAT                                  },                  
    { GL_RG8UI,          GL_RG_INTEGER,  GL_UNSIGNED_BYTE                          },          
    { GL_RG8I,           GL_RG_INTEGER,  GL_BYTE                                   },           
    { GL_RG16UI,         GL_RG_INTEGER,  GL_UNSIGNED_SHORT                         },         
    { GL_RG16I,          GL_RG_INTEGER,  GL_SHORT                                  },          
    { GL_RG32UI,         GL_RG_INTEGER,  GL_UNSIGNED_INT                           },         
    { GL_RG32I,          GL_RG_INTEGER,  GL_INT                                    },          
    { GL_R8,             GL_RED,         GL_UNSIGNED_BYTE                          },                    
    { GL_R8_SNORM,       GL_RED,         GL_BYTE                                   },  
    //We use GL_FLOATS for this one                                               
    //{ GL_R16F,           GL_RED,         GL_HALF_FLOAT                           },                  
    { GL_R32F,           GL_RED,         GL_FLOAT                                  },                  
    { GL_R16F,           GL_RED,         GL_FLOAT                                  },                  
    { GL_R8UI,           GL_RED_INTEGER, GL_UNSIGNED_BYTE                          },          
    { GL_R8I,            GL_RED_INTEGER, GL_BYTE                                   },           
    { GL_R16UI,          GL_RED_INTEGER, GL_UNSIGNED_SHORT                         },         
    { GL_R16I,           GL_RED_INTEGER, GL_SHORT                                  },          
    { GL_R32UI,          GL_RED_INTEGER, GL_UNSIGNED_INT                           },         
    { GL_R32I,           GL_RED_INTEGER, GL_INT                                    },          
    { GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT                 },
    { GL_DEPTH_COMPONENT24,  GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                   },
    { GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                   },
    { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT                          },
    { GL_DEPTH24_STENCIL8,   GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8              },
    { GL_DEPTH32F_STENCIL8,  GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV },
    { GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE                     },
    //{ GL_RGBA,            GL_RGBA,            GL_UNSIGNED_SHORT_4_4_4_4          },
    //{ GL_RGBA,            GL_RGBA,            GL_UNSIGNED_SHORT_5_5_5_1          },
    { GL_RGB,             GL_RGB,             GL_UNSIGNED_BYTE                     },
    //{ GL_RGB,             GL_RGB,             GL_UNSIGNED_SHORT_5_6_5            },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE                     },
    { GL_LUMINANCE,       GL_LUMINANCE,       GL_UNSIGNED_BYTE                     },
    { GL_ALPHA,           GL_ALPHA,           GL_UNSIGNED_BYTE                     },

    //DT GL only (TODO)

    { GL_RGBA16,          GL_RGBA,            GL_UNSIGNED_SHORT                    },
    { GL_RGB16,           GL_RGB,             GL_UNSIGNED_SHORT                    },
    { GL_RG16,            GL_RG,              GL_UNSIGNED_SHORT                    },
    { GL_R16,             GL_RED,             GL_UNSIGNED_SHORT                    },

    { GL_R3_G3_B2,        GL_RGB,             GL_UNSIGNED_BYTE_3_3_2               },

    { GL_RGB4,              GL_RGBA,          GL_UNSIGNED_SHORT_4_4_4_4            }, //RGBA instead of RGB
    { GL_RGB5,              GL_RGBA,          GL_UNSIGNED_SHORT_5_5_5_1            }, //RGBA instead of RGB
    { GL_RGB10,             GL_RGBA,          GL_UNSIGNED_INT_2_10_10_10_REV       }, //RGBA instead of RGB
    //Cannot reliably support this. fallback to floats or ubytes...
    //{ GL_RGB12,           GL_RGB,           ?                                    },

    //Cannot reliably support this. fallback to floats or ubytes...                
    //{ GL_RGBA2, GL_RGBA, ?                                                       },  
    //Cannot reliably support this. fallback to floats or ubytes...                
    //{ GL_RGBA12, GL_RGBA, ?                                                      },  

    { GL_RGBA16_SNORM,      GL_RGBA,            GL_SHORT                           },
    { GL_RGB16_SNORM,       GL_RGB,             GL_SHORT                           },
    { GL_RG16_SNORM,        GL_RG,              GL_SHORT                           },
    { GL_R16_SNORM,         GL_RED,             GL_SHORT                           },

    { GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT                    },

    { GL_ALPHA,            GL_ALPHA,            GL_UNSIGNED_BYTE                   },

    //Cannot reliably support this. fallback to floats or ubytes...
    { GL_ALPHA4,           GL_ALPHA,            GL_FLOAT                           },
    { GL_ALPHA8,           GL_ALPHA,            GL_UNSIGNED_BYTE                   },
    { GL_ALPHA12,          GL_ALPHA,            GL_FLOAT                           },
    { GL_ALPHA16,          GL_ALPHA,            GL_UNSIGNED_SHORT                  },

    { GL_LUMINANCE,           GL_LUMINANCE,          GL_UNSIGNED_BYTE              },
    { GL_LUMINANCE4,          GL_LUMINANCE,          GL_FLOAT                      },
    { GL_LUMINANCE8,          GL_LUMINANCE,          GL_UNSIGNED_BYTE              },
    { GL_LUMINANCE12,         GL_LUMINANCE,          GL_FLOAT                      },
    { GL_LUMINANCE16,         GL_LUMINANCE,          GL_UNSIGNED_SHORT             },
    { GL_LUMINANCE_ALPHA,     GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE              },
    { GL_LUMINANCE4_ALPHA4,   GL_LUMINANCE_ALPHA,    GL_FLOAT                      },
    { GL_LUMINANCE6_ALPHA2,   GL_LUMINANCE_ALPHA,    GL_FLOAT                      },
    { GL_LUMINANCE8_ALPHA8,   GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE              },
    { GL_LUMINANCE12_ALPHA4,  GL_LUMINANCE_ALPHA,    GL_FLOAT                      },
    { GL_LUMINANCE12_ALPHA12, GL_LUMINANCE_ALPHA,    GL_FLOAT                      },
    { GL_LUMINANCE16_ALPHA16, GL_LUMINANCE_ALPHA,    GL_UNSIGNED_SHORT             },

    { GL_SLUMINANCE,          GL_LUMINANCE,          GL_UNSIGNED_BYTE              },
    { GL_SLUMINANCE8,         GL_LUMINANCE,          GL_UNSIGNED_BYTE              },
    { GL_SLUMINANCE_ALPHA,    GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE              },
    { GL_SLUMINANCE8_ALPHA8,  GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE              },

    { GL_INTENSITY,           GL_ALPHA,              GL_UNSIGNED_BYTE              },
    { GL_INTENSITY4,          GL_ALPHA,              GL_FLOAT                      },
    { GL_INTENSITY8,          GL_ALPHA,              GL_UNSIGNED_BYTE              },
    { GL_INTENSITY12,         GL_ALPHA,              GL_FLOAT                      },
    { GL_INTENSITY16,         GL_ALPHA,              GL_UNSIGNED_SHORT             },

    { GL_STENCIL_INDEX8,      GL_STENCIL_INDEX,      GL_UNSIGNED_BYTE              },

};


GLInternalFormat* GLFormats::getInternalFormat(GLenum internalFormat) {
    for (size_t i = 0; i < sizeof(g_InternalFormats)/sizeof(g_InternalFormats[0]); i++) {
        if (g_InternalFormats[i].internalFormat == internalFormat) {
            return &g_InternalFormats[i];
        }
    }
    return NULL;
}

GLDataFormat* GLFormats::getDataFormat(GLenum dataFormat) {
    for (size_t i = 0; i < sizeof(g_DataFormats)/sizeof(g_DataFormats[0]); i++) {
        if (g_DataFormats[i].format == dataFormat) {
            return &g_DataFormats[i];
        }
    }
    return NULL;
}

GLDataType* GLFormats::getDataType(GLenum dataType) {
    for (size_t i = 0; i < sizeof(g_DataTypes)/sizeof(g_DataTypes[0]); i++) {
        if (g_DataTypes[i].type == dataType) {
            return &g_DataTypes[i];
        }
    }
    return NULL;
}

DGLPixelTransfer::DGLPixelTransfer(std::vector<GLint> _rgbaSizes, std::vector<GLint> _depthStencilSizes, GLenum internalFormat):m_DataFormat(NULL),m_DataType(NULL) {

    //try recongnize used internalformat
    GLInternalFormat* internalFormatDesr = GLFormats::getInternalFormat(internalFormat);

    if (!internalFormatDesr) {

        //internal format was not given, or we do not have hardcoded (format, type) for it)/
        //try quess them            

        std::vector<GLint> rgbaSizes(_rgbaSizes);
        rgbaSizes.resize(4, 0);

        bool isColorBuffer = false;
        for (size_t i = 0; i < rgbaSizes.size(); i++)
            isColorBuffer |= (rgbaSizes[i] != 0);

        if (isColorBuffer) {

            if (rgbaSizes[3]) {
                m_DataFormat = GLFormats::getDataFormat(GL_RGBA);
            } else if (rgbaSizes[2]) {
                m_DataFormat = GLFormats::getDataFormat(GL_RGB);
            } else if (rgbaSizes[1]) {
                m_DataFormat = GLFormats::getDataFormat(GL_RG);
            } else if (rgbaSizes[0]) {
                m_DataFormat = GLFormats::getDataFormat(GL_RED);
            } else {
                //the buffer is probably empty.
                return;
            }
            int minSize = 0; 
            for (int i = 0; i < 4; i++) {
                minSize = MIN(minSize, rgbaSizes[i]);
            }
            if (minSize > 16) {
                m_DataType = GLFormats::getDataType(GL_UNSIGNED_INT);
            } else if (minSize > 8) {
                m_DataType = GLFormats::getDataType(GL_UNSIGNED_SHORT);
            } else {
                m_DataType = GLFormats::getDataType(GL_UNSIGNED_BYTE);
            }

        } else {
            std::vector<GLint> depthStencilSizes(_depthStencilSizes);
            depthStencilSizes.resize(2, 0);

            if (depthStencilSizes[0] == 0 && depthStencilSizes[1] == 0) {
                //the buffer is probably empty.
                return;
            }

            if (depthStencilSizes[0] != 0 && depthStencilSizes[1] == 0) {

                m_DataFormat = GLFormats::getDataFormat(GL_DEPTH_COMPONENT);
                m_DataType = GLFormats::getDataType(GL_FLOAT);

            } else if (depthStencilSizes[0] == 0 && depthStencilSizes[1] != 0) {

                m_DataFormat = GLFormats::getDataFormat(GL_STENCIL_INDEX);
                m_DataType = GLFormats::getDataType(GL_UNSIGNED_BYTE);

            } else {

                m_DataFormat = GLFormats::getDataFormat(GL_DEPTH_STENCIL);
                m_DataType = GLFormats::getDataType(GL_UNSIGNED_INT_24_8);

            }
        }
    } else {
        m_DataFormat = GLFormats::getDataFormat(internalFormatDesr->dataFormat);
        m_DataType = GLFormats::getDataType(internalFormatDesr->dataType);
    }
} 


bool DGLPixelTransfer::isValid() {
    return m_DataFormat && m_DataType;;
}


GLenum DGLPixelTransfer::getFormat() {
    if (!isValid())
        throw std::runtime_error("DGLPixelTransfer::getFormat called, but pixel transfer is not valid.");
    return m_DataFormat->format;
}

GLenum DGLPixelTransfer::getType() {
    if (!isValid())
        throw std::runtime_error("DGLPixelTransfer::getFormat called, but pixel transfer is not valid.");
    return m_DataType->type;
}

unsigned int DGLPixelTransfer::getPixelSize() {
    if (!isValid()) {
        return 0;
    } else {
        if (m_DataType->packed) {
            return m_DataType->byteSize;
        } else {
            return m_DataFormat->components * m_DataType->byteSize;
        }
    }
}