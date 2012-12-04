#ifndef GL_SERIALIZED_H
#define GL_SERIALIZED_H

#include <DGLCommon/gl-types.h>
#include <DGLNet/serializer-fwd.h>

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

#include <vector>

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
    operator TBase*() const {return static_cast<TBase*>(m_value);}
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
    GLenumWrap():m_value(NULL) {}

    GLenumWrap(uint64_t v):m_value(v) {}

    uint64_t get() { return m_value; }
    operator GLenum() const { return static_cast<GLenum>(m_value); }
private:
    uint64_t m_value;
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
    void get(const TBase*& v) const { v = (const TBase*)boost::get<PtrWrap<const void*>>(m_value); }
    template<typename TBase>
    void get(TBase*& v) const { v = (TBase*)boost::get<PtrWrap<void*>>(m_value); }
    //for function pointers const qualifier is meaningless, so we need specific overload to resolve ambiguity
    void get(PROC& v) const { v = (PROC)boost::get<PtrWrap<const void*>>(m_value); }

    std::string toString() const;

private:
    boost::variant<signed long long, unsigned long long, signed long, unsigned long, unsigned int, signed int, unsigned short, signed short, unsigned char, signed char, float, double,
        PtrWrap<void*>, PtrWrap<const void*>, GLenumWrap> m_value;
};

class CalledEntryPoint {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_args;
        ar & m_entryp;
        ar & m_glError;
        ar & m_DebugOutput;
    }

public:
    CalledEntryPoint() {}
    CalledEntryPoint(Entrypoint, int numArgs);
    Entrypoint getEntrypoint() const;
    void setError(uint32_t error);
    void setDebugOutput(const std::string& message);

    const std::vector<AnyValue>& getArgs() const;
    template<typename T>
    void operator << (const T& arg) {
        m_args[m_SavedArgsCount] = arg;
        m_SavedArgsCount++;
    }
    std::string toString() const;
private: 
    std::vector<AnyValue> m_args;
    Entrypoint m_entryp;
    int32_t m_glError;
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
        ObjectTypeGPU
    };
};

class DGLResourceTexture: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_Levels;
    }

public:

    class TextureLevel {
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & m_Width;
            ar & m_Height;
            ar & m_Channels;
            ar & m_Pixels;
        }
    public:
        int32_t m_Width, m_Height, m_Channels;
        std::vector<int8_t> m_Pixels;
    };

    std::vector<TextureLevel> m_Levels;
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
        ar & m_Width;
        ar & m_Height;
        ar & m_Channels;
        ar & m_Pixels;
    }

public:

    int32_t m_Width, m_Height, m_Channels;
    std::vector<int8_t> m_Pixels;
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
            ar & m_Width;
            ar & m_Height;
            ar & m_Channels;
            ar & m_Pixels;
            ar & m_Id;
        }
    public:
        FBOAttachment() {}
        FBOAttachment(uint32_t id);

        void error(std::string msg);
        bool isOk(std::string& error) const;


        int32_t m_Width, m_Height, m_Channels;
        std::vector<int8_t> m_Pixels;
        uint32_t m_Id;
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
    std::pair<std::string, uint32_t> m_CompileStatus;
};

class DGLResourceProgram: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & mLinkStatus;
        ar & m_AttachedShaders;
    }

public:

    std::pair<std::string, uint32_t> mLinkStatus;
    std::vector<std::pair<uint32_t, uint32_t>> m_AttachedShaders;
};

class DGLResourceGPU: public DGLResource {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<DGLResource>(*this);
        ar & m_Renderer;
        ar & m_Version;
        ar & m_Vendor;
    }


public:
    std::string m_Renderer, m_Version, m_Vendor;
};

#endif