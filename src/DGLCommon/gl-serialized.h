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


#ifndef GL_SERIALIZED_H
#define GL_SERIALIZED_H

#include <DGLCommon/gl-types.h>
#include <DGLNet/serializer-fwd.h>

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

typedef int (*FUNC_PTR) ();

//Pointers that are serialized by value must be wrapped with this class
template <typename T>
class PtrWrap {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        long long tmp = reinterpret_cast<long long>(m_value);
        ar & tmp;
        m_value = reinterpret_cast<T>(tmp);
    }
public:
    PtrWrap():m_value(NULL) {}
    
    template<typename TCast>
    PtrWrap(TCast v):m_value((T)v) {}

    template<typename TBase>
    operator TBase*() const {return reinterpret_cast<TBase*>(m_value);}
private:
    T m_value;
};

class GLenumWrap {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_value;
    }

public:
    GLenumWrap():m_value(0) {}

    GLenumWrap(gl_t v):m_value(v) {}

    gl_t get() { return m_value; }
    operator GLenum() const { return static_cast<GLenum>(m_value); }
private:
    gl_t m_value;
};


class AnyValue {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_value;
    }

public:
    AnyValue() {};
    template<typename T>
    AnyValue(T v):m_value(v) {}

    template<typename TBase>
    AnyValue(const TBase* v):m_value(PtrWrap<const void*>((const void*)v)) {}
    template<typename TBase>
    AnyValue(TBase* v):m_value(PtrWrap<void*>((void*)v)) {}

    template<typename T>
    void get(T& v) const { v = boost::get<T>(m_value); }
    template<typename TBase>
    void get(const TBase*& v) const { v = (const TBase*)boost::get<PtrWrap<const void*> >(m_value); }
    template<typename TBase>
    void get(TBase*& v) const { v = (TBase*)boost::get<PtrWrap<void*> >(m_value); }
    //for function pointers const qualifier is meaningless, so we need specific overload to resolve ambiguity
    void get(FUNC_PTR& v) const { v = (FUNC_PTR)boost::get<PtrWrap<const void*> >(m_value); }

    void writeToSS(std::ostringstream& out) const;

private:
    boost::variant<signed long long, unsigned long long, signed long, unsigned long, unsigned int, signed int, unsigned short, signed short, unsigned char, signed char, float, double,
        PtrWrap<void*>, PtrWrap<const void*>, GLenumWrap> m_value;
};

class RetValue: public AnyValue {
public: 
    RetValue():m_isSet(false) {}

    RetValue(const RetValue& v):AnyValue(*static_cast<const AnyValue*>(&v)),m_isSet(v.isSet()) {}

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<AnyValue>(*this);;
        ar & m_isSet;
    }

    template<typename T>
    RetValue(T v):AnyValue(v),m_isSet(true) {}

    bool isSet() const { return m_isSet; }   
private: 
    bool m_isSet;
};

class CalledEntryPoint {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_args;
        ar & m_retVal;
        ar & m_entryp;
        ar & m_glError;
        ar & m_DebugOutput;
    }

public:
    CalledEntryPoint() {}
    CalledEntryPoint(Entrypoint, int numArgs);
    Entrypoint getEntrypoint() const;
    void setRetVal(const RetValue& ret);
    void setError(gl_t error);
    void setDebugOutput(const std::string& message);

    const std::vector<AnyValue>& getArgs() const;
    template<typename T>
    void operator << (const T& arg) {
        m_args[m_SavedArgsCount] = arg;
        m_SavedArgsCount++;
    }
    std::string toString() const;
    const RetValue& getRetVal() const;
    gl_t getError() const;
    const std::string& getDebugOutput() const;
private: 
    std::vector<AnyValue> m_args;
    RetValue m_retVal;
    Entrypoint m_entryp;
    gl_t m_glError;
    std::string m_DebugOutput;
    int m_SavedArgsCount;
};

class DGLResource {
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {}

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

class DGLPixelRectangle {
    friend class boost::serialization::access;

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

namespace boost { namespace serialization {
    template<class Archive>
    inline void save_construct_data(
        Archive & ar, const DGLPixelRectangle * t, const unsigned int file_version) {
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
        Archive & ar, DGLPixelRectangle * t, const unsigned int file_version) {
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
            ::new(t)DGLPixelRectangle(width, height, rowBytes, glFormat, glType, iformat, samples);
    }
}}

class DGLResourceTexture: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_FacesLevels;
    }

public:
    std::vector<std::vector<boost::shared_ptr<DGLPixelRectangle> > > m_FacesLevels;
};

class DGLResourceBuffer: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_Data;
    }

public:
    std::vector<char> m_Data;
};

class DGLResourceFramebuffer: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_PixelRectangle;
    }

public:

    boost::shared_ptr<DGLPixelRectangle> m_PixelRectangle;
};

class DGLResourceFBO: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_Attachments;
    }

public:
    class FBOAttachment {
        friend class boost::serialization::access;

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

        boost::shared_ptr<DGLPixelRectangle> m_PixelRectangle;
        gl_t m_Id;
    private:
        bool m_Ok;
        std::string m_ErrorMsg;
    };

    std::vector<FBOAttachment> m_Attachments;
};

class DGLResourceShader: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_Sources;
        ar & m_CompileStatus;
    }

public:
    std::vector<std::string> m_Sources;
    std::pair<std::string, gl_t> m_CompileStatus;
};

class DGLResourceProgram: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
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
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
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
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
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

class ContextObjectName {
public:
    ContextObjectName();
    ContextObjectName(opaque_id_t context, gl_t name, gl_t target = 0);
    virtual ~ContextObjectName();
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_Name;
        ar & m_Context;
        ar & m_Target;
    }

    virtual bool operator==(const ContextObjectName&rhs) const;

    virtual bool operator<(const ContextObjectName&rhs) const;

    gl_t m_Name;
    opaque_id_t m_Context;
    gl_t m_Target;
};

#endif
