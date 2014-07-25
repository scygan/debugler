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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <boost/shared_ptr.hpp>
#include <vector>

#include <DGLNet/protocol/msgutils.h>
#include <DGLNet/protocol/anyvalue.h>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>

namespace dglnet {

class DGLResource : public message::utils::ReplyBase {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<message::utils::ReplyBase>(
            *this);
    }

    virtual ~DGLResource() {}
};

class DGLBenchmarkBuffer : public message::utils::ReplyBase {
public:
    template <class Archive>
    void save(Archive& ar, const unsigned int) const {
        ar& boost::serialization::base_object<message::utils::ReplyBase>(*this);
        ar& m_Size;
        boost::serialization::binary_object bo(m_Buffer, m_Size);
        ar& bo;
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<message::utils::ReplyBase>(*this);
        ar& m_Size;
        m_Buffer = static_cast<char*>(malloc(m_Size));
        boost::serialization::binary_object bo(m_Buffer, m_Size);
        ar& bo;
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        boost::serialization::split_member(ar, *this, version);
    }

    DGLBenchmarkBuffer() : m_Size(0) {}
    DGLBenchmarkBuffer(value_t size) {
        m_Size = size;

        m_Buffer = static_cast<char*>(malloc(size));

        for (value_t i = 0; i < m_Size; i++) {
            m_Buffer[i] = static_cast<char>(rand());
        }
    }

    virtual ~DGLBenchmarkBuffer() {
        free(m_Buffer);
    }

    char* m_Buffer;
    value_t m_Size;
};

namespace resource {

class DGLPixelRectangle {
public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        // m_StorageSize is size of underlying object now (on both load() and
        // save())
        boost::serialization::binary_object bo(m_Storage,
                                               m_Height * m_RowBytes);
        ar& bo;
    }

    /**
     * Ctor
     *
     * @param width real width of pixel rectangle
     * @param height real height of pixel rectangle
     * @param rowBytes byte with of pixel rectangle (including alignment)
     * @param glFormat GL data format of sent data
     * @param glType GL data type of sent data
     */
    DGLPixelRectangle(value_t width, value_t height, value_t rowBytes,
                      gl_t glFormat, gl_t glType);
    DGLPixelRectangle(const DGLPixelRectangle& rhs);
    ~DGLPixelRectangle();

    value_t m_Width, m_Height, m_RowBytes;
    gl_t m_GLFormat, m_GLType;

    void* getPtr() const;
    size_t getSize() const;

   private:
    void* m_Storage;
};

class DGLResourceTexture : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_FacesLevelsLayers;
        ar& m_Target;
    }

    class TextureLayer {
    public:
        template <class Archive>
        void serialize(Archive& ar, const unsigned int) {
            ar& m_PixelRectangle;
            ar& m_InternalFormat;
            ar& m_Samples;
        }
        ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle>
            m_PixelRectangle;
        gl_t m_InternalFormat;
        value_t m_Samples;
    };

    std::vector<std::vector<std::vector<TextureLayer> > > m_FacesLevelsLayers;

    gl_t m_Target;
};

class DGLResourceBuffer : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_Data;
    }

    std::vector<char> m_Data;
};

class DGLResourceFramebuffer : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_PixelRectangle;
    }

    ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle> m_PixelRectangle;
};

class DGLResourceFBO : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_Attachments;
        ar& m_CompletenessStatus;
    }


    class FBOAttachment {

    public:
        template <class Archive>
        void serialize(Archive& ar, const unsigned int) {
            ar& m_Ok;
            ar& m_ErrorMsg;
            ar& m_PixelRectangle;
            ar& m_Id;
            ar& m_Internalformat;
            ar& m_Samples;
        }

        FBOAttachment() {}
        FBOAttachment(gl_t id);

        void error(std::string msg);
        bool isOk(std::string& error) const;

        ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle>
                m_PixelRectangle;
        gl_t m_Id;
        gl_t m_Internalformat;
        value_t m_Samples;

       private:
        bool m_Ok;
        std::string m_ErrorMsg;
    };

    std::vector<FBOAttachment> m_Attachments;
    gl_t                       m_CompletenessStatus;
};

class DGLResourceRenderbuffer : public DGLResource {
public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_PixelRectangle;
        ar& m_Internalformat;
        ar& m_Samples;
    }

    ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle>
        m_PixelRectangle;

    gl_t m_Internalformat;
    value_t m_Samples;

};

class DGLResourceShader : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_Source;
        ar& m_IsESSLDefault;
        ar& m_CompileStatus;
    }

    std::string m_Source;
    bool m_IsESSLDefault;
    std::pair<std::string, gl_t> m_CompileStatus;
};

class DGLResourceProgram : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& mLinkStatus;
        ar& m_AttachedShaders;
        ar& m_Uniforms;
        ar& m_EmbeddedSSOSource;
        ar& m_EmbeddedSSOSourceIsESSL;
    }

    struct Uniform {
        gl_t m_type;
        value_t m_rowSize, m_location;
        std::string m_name;
        std::vector<AnyValue> m_value;
        bool m_supportedType;

        template <class Archive>
        void serialize(Archive& ar, const unsigned int) {
            ar& m_type;
            ar& m_location;
            ar& m_name;
            ar& m_value;
            ar& m_supportedType;
            ar& m_rowSize;
        }
    };

    std::pair<std::string, gl_t> mLinkStatus;
    std::vector<std::pair<gl_t, gl_t> > m_AttachedShaders;
    std::vector<Uniform> m_Uniforms;

    /** 
     * GLSL source for embedded SSO (glCreateShaderProgram)
     */
    std::string m_EmbeddedSSOSource;
    bool m_EmbeddedSSOSourceIsESSL;
};

class DGLResourceGPU : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_Renderer;
        ar& m_Version;
        ar& m_GLSL;
        ar& m_Vendor;
        ar& m_hasNVXGPUMemoryInfo;
        ar& m_nvidiaMemory;
    }

    struct NVXGPUMemoryInfo {

        template <class Archive>
        void serialize(Archive& ar, const unsigned int) {
            ar& memInfoDedidactedVidMem;
            ar& memInfoTotalAvailMem;
            ar& memInfoCurrentAvailVidMem;
            ar& memInfoEvictionCount;
            ar& memInfoEvictedMem;
        }

        value_t memInfoDedidactedVidMem;
        value_t memInfoTotalAvailMem;
        value_t memInfoCurrentAvailVidMem;
        value_t memInfoEvictionCount;
        value_t memInfoEvictedMem;
    };

    std::string m_Renderer, m_Version, m_Vendor, m_GLSL;
    bool m_hasNVXGPUMemoryInfo;
    NVXGPUMemoryInfo m_nvidiaMemory;
};

class DGLResourceBacktrace : public DGLResource {
public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_trace;
    }
    std::vector<std::string> m_trace;
};

namespace utils {
    class StateItem {
       public:
        template <class Archive>
        void serialize(Archive& ar, const unsigned int) {
            ar& m_Name;
            ar& m_Values;
        }
        std::string m_Name;
        std::vector<AnyValue> m_Values;
    };
}

class DGLResourceState : public DGLResource {
   public:
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& ::boost::serialization::base_object<DGLResource>(*this);
        ar& m_Items;
    }
   public:
    std::vector<utils::StateItem> m_Items;
};

}    // namespace resource
}    // namespace dglnet

namespace boost {
namespace serialization {
template <class Archive>
inline void save_construct_data(Archive& ar,
                                const dglnet::resource::DGLPixelRectangle* t,
                                const unsigned int /*version*/) {
    // save data required to construct instance
    ar << t->m_Width;
    ar << t->m_Height;
    ar << t->m_RowBytes;
    ar << t->m_GLFormat;
    ar << t->m_GLType;
}

template <class Archive>
inline void load_construct_data(Archive& ar,
                                dglnet::resource::DGLPixelRectangle* t,
                                const unsigned int /*version*/) {
    // retrieve data from archive required to construct new instance
    value_t width, height, rowBytes, glFormat, glType;
    ar >> width;
    ar >> height;
    ar >> rowBytes;
    ar >> glFormat;
    ar >> glType;
    // invoke inplace constructor
    ::new (t) dglnet::resource::DGLPixelRectangle(
            width, height, rowBytes, glFormat, glType);
}
}
}

#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::DGLResource,                       dR)
REGISTER_CLASS(dglnet::DGLBenchmarkBuffer,                dBBR)
REGISTER_CLASS(dglnet::resource::DGLResourceTexture,      dsRT)
REGISTER_CLASS(dglnet::resource::DGLResourceBuffer,       dsRB)
REGISTER_CLASS(dglnet::resource::DGLResourceFramebuffer,  dsRFB)
REGISTER_CLASS(dglnet::resource::DGLResourceFBO,          dsFBO)
REGISTER_CLASS(dglnet::resource::DGLResourceRenderbuffer, dsRBO)
REGISTER_CLASS(dglnet::resource::DGLResourceShader,       dsRSH)
REGISTER_CLASS(dglnet::resource::DGLResourceProgram,      dsRP)
REGISTER_CLASS(dglnet::resource::DGLResourceGPU,          dsRGPU)
REGISTER_CLASS(dglnet::resource::DGLResourceState,        dsRS)
REGISTER_CLASS(dglnet::resource::DGLResourceBacktrace,    dsRBT)
#endif

#endif    // RESOURCE_H
