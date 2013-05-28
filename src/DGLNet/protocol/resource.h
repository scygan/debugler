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

#include <DGLNet/protocol/anyvalue.h>
#include <DGLNet/protocol/message.h>
#include <boost/serialization/binary_object.hpp>

namespace dglnet {

class DGLResource: public message::RequestReply::ReplyBase {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<message::RequestReply::ReplyBase>(*this);
    }

public:
    virtual ~DGLResource() {}

    enum ObjectType {
        ObjectTypeTexture,
        ObjectTypeFBO,
        ObjectTypeFramebuffer,
        ObjectTypeShader,
        ObjectTypeProgram,
        ObjectTypeBuffer,
        ObjectTypeGPU,
        ObjectTypeState,
    };
};

namespace resource {
    
class DGLPixelRectangle {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        //m_StorageSize is size of underlying object now (on both load() and save())
        boost::serialization::binary_object bo(m_Storage, m_Height * m_RowBytes);
        ar & bo;
    }
public:
    /**
     * Ctor
     *
     * @param width real width of pixel rectangle
     * @param height real height of pixel rectangle
     * @param rowBytes byte with of pixel rectangle (including alignment)
     * @param glFormat GL data format of sent data
     * @param glType GL data type of sent data
     * @param iFormat storage internal format (for informational purposes only)
     */
    DGLPixelRectangle(value_t width, value_t height, value_t rowBytes, gl_t glFormat, gl_t glType, gl_t iFormat, value_t samples);
    DGLPixelRectangle(const DGLPixelRectangle& rhs);
    ~DGLPixelRectangle();

    value_t m_Width, m_Height, m_RowBytes, m_Samples;
    gl_t  m_GLFormat, m_GLType, m_InternalFormat;

    void* getPtr() const;
    size_t getSize() const;

private:
    void* m_Storage;
};

class DGLResourceTexture: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_FacesLevels;
    }

public:
    std::vector<std::vector<::boost::shared_ptr<dglnet::resource::DGLPixelRectangle> > > m_FacesLevels;
};

class DGLResourceBuffer: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_Data;
    }

public:
    std::vector<char> m_Data;
};

class DGLResourceFramebuffer: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_PixelRectangle;
    }

public:

    ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle> m_PixelRectangle;
};

class DGLResourceFBO: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_Attachments;
    }

public:
    class FBOAttachment {
        friend class ::boost::serialization::access;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & m_Ok;
            ar & m_ErrorMsg;
            ar & m_PixelRectangle;
            ar & m_Id;
        }
    public:
        FBOAttachment() {}
        FBOAttachment(gl_t id);

        void error(std::string msg);
        bool isOk(std::string& error) const;

        ::boost::shared_ptr<dglnet::resource::DGLPixelRectangle> m_PixelRectangle;
        gl_t m_Id;
    private:
        bool m_Ok;
        std::string m_ErrorMsg;
    };

    std::vector<FBOAttachment> m_Attachments;
};

class DGLResourceShader: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_Source;
        ar & m_ShaderObjDeleted;
        ar & m_CompileStatus;
    }

public:
    std::string m_Source;
    bool m_ShaderObjDeleted;
    std::pair<std::string, gl_t> m_CompileStatus;
};

class DGLResourceProgram: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & mLinkStatus;
        ar & m_AttachedShaders;
        ar & m_Uniforms;
    }

public:

    struct Uniform {
        gl_t m_type;
        value_t m_rowSize, m_location;
        std::string m_name;
        std::vector<AnyValue> m_value;
        bool m_supportedType;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & m_type;
            ar & m_location;
            ar & m_name;
            ar & m_value;
            ar & m_supportedType;
            ar & m_rowSize;
        }
    };

    std::pair<std::string, gl_t> mLinkStatus;
    std::vector<std::pair<gl_t, gl_t> > m_AttachedShaders;
    std::vector<Uniform> m_Uniforms;
};

class DGLResourceGPU: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_Renderer;
        ar & m_Version;
        ar & m_GLSL;
        ar & m_Vendor;
        ar & m_hasNVXGPUMemoryInfo;
        ar & m_nvidiaMemory;
    }

    struct NVXGPUMemoryInfo {

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & memInfoDedidactedVidMem;
            ar & memInfoTotalAvailMem;
            ar & memInfoCurrentAvailVidMem;
            ar & memInfoEvictionCount;
            ar & memInfoEvictedMem;
        }

        value_t memInfoDedidactedVidMem;
        value_t memInfoTotalAvailMem;
        value_t memInfoCurrentAvailVidMem;
        value_t memInfoEvictionCount;
        value_t memInfoEvictedMem;
    };


public:
    std::string m_Renderer, m_Version, m_Vendor, m_GLSL;
    bool m_hasNVXGPUMemoryInfo;
    NVXGPUMemoryInfo m_nvidiaMemory;
};


class DGLResourceState: public DGLResource {
    friend class ::boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & ::boost::serialization::base_object<DGLResource>(*this);
        ar & m_Items;
    }

public:
    struct StateItem {

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & m_Name;
            ar & m_Values;
        }
        std::string m_Name;
        std::vector<AnyValue> m_Values;
    };


public:
    std::vector<StateItem> m_Items;
};

} //namespace resource
} //namespace dglnet


namespace boost { namespace serialization {
    template<class Archive>
    inline void save_construct_data(
        Archive & ar, const dglnet::resource::DGLPixelRectangle * t, const unsigned int file_version) {
            // save data required to construct instance
            ar << t->m_Width;
            ar << t->m_Height;
            ar << t->m_RowBytes;
            ar << t->m_GLFormat;
            ar << t->m_GLType;
            ar << t->m_InternalFormat;
            ar << t->m_Samples;
    }

    template<class Archive>
    inline void load_construct_data(
        Archive & ar, dglnet::resource::DGLPixelRectangle * t, const unsigned int file_version) {
            // retrieve data from archive required to construct new instance
            value_t width, height, rowBytes, glFormat, glType, samples;
            gl_t iformat;
            ar >> width;
            ar >> height;
            ar >> rowBytes;
            ar >> glFormat;
            ar >> glType;
            ar >> iformat;
            ar >> samples;
            // invoke inplace constructor
            ::new(t)dglnet::resource::DGLPixelRectangle(width, height, rowBytes, glFormat, glType, iformat, samples);
    }
}}

#ifdef REGISTER_CLASS
REGISTER_CLASS(dglnet::DGLResource);
REGISTER_CLASS(dglnet::resource::DGLResourceTexture);
REGISTER_CLASS(dglnet::resource::DGLResourceBuffer);
REGISTER_CLASS(dglnet::resource::DGLResourceFramebuffer);
REGISTER_CLASS(dglnet::resource::DGLResourceFBO);
REGISTER_CLASS(dglnet::resource::DGLResourceShader);
REGISTER_CLASS(dglnet::resource::DGLResourceProgram);
REGISTER_CLASS(dglnet::resource::DGLResourceGPU);
REGISTER_CLASS(dglnet::resource::DGLResourceState);
#endif


#endif //RESOURCE_H
