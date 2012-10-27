#include "DGLCommon/gl-types.h"
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>
#include <vector>

class AnyValue {
public:
    AnyValue() {};
    template<typename T>
    AnyValue(T v):m_value(v) {}

    //template<typename T>
    //AnyValue(const AnyValue& v):m_value(v.get<T>()) {}

    template<typename T>
    operator T() const { return get<T>(); }

protected:
    template<typename T>
    T get() const { return boost::get<T>(m_value); }

private:
    typedef int (WINAPI *NativeEntrpType)(void);

    boost::variant<signed long long, unsigned long long, signed long, unsigned long, unsigned int, signed int, unsigned short, signed short, unsigned char, signed char, float, double, void*, const void*, NativeEntrpType, HGLRC> m_value;
};

class RetValue: public AnyValue {
public: 
   RetValue():m_isSet(false) {}

   RetValue(const RetValue& v):AnyValue(*static_cast<const AnyValue*>(&v)),m_isSet(v.isSet()) {}

   template<typename T>
   RetValue(T v):AnyValue(v),m_isSet(true) {}

   template<typename T>
   operator T() const { return get<T>(); }

   bool isSet() const { return m_isSet; }   
private: 
    bool m_isSet;
};



class ITracer {
public: 
    virtual RetValue Pre(Entrypoint, const std::vector<AnyValue>&) = 0; 
    virtual void Post(Entrypoint) = 0; 
};


class DefaultTracer: public ITracer {
protected:
    virtual RetValue Pre(Entrypoint, const std::vector<AnyValue>& ); 
    virtual void Post(Entrypoint call);
};

class GetProcAddressTracer: public DefaultTracer {
    virtual RetValue Pre(Entrypoint, const std::vector<AnyValue>& ); 
    virtual void Post(Entrypoint call);
};



extern boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];

template<typename Tracer>
void SetAllTracers() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Tracers[i] = boost::shared_ptr<ITracer>(new Tracer());
    }
}

template<typename Tracer>
void SetTracer(Entrypoint entrp) {
    g_Tracers[entrp] = boost::shared_ptr<ITracer>(new Tracer());
}


