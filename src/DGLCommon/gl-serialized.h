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

private:
    boost::variant<signed long long, unsigned long long, signed long, unsigned long, unsigned int, signed int, unsigned short, signed short, unsigned char, signed char, float, double,
        PtrWrap<void*>, PtrWrap<const void*> > m_value;
};

class CalledEntryPoint {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & m_args;
        ar & m_entryp;
    }

public:
    CalledEntryPoint() {}
    CalledEntryPoint(Entrypoint, int numArgs);
    Entrypoint getEntrypoint() const;
    const std::vector<AnyValue>& getArgs() const;
    template<typename T>
    void operator << (const T& arg) {
        m_args[m_SavedArgsCount] = arg;
        m_SavedArgsCount++;
    }
private: 
    std::vector<AnyValue> m_args;
    Entrypoint m_entryp;
    int m_SavedArgsCount;
};
#endif