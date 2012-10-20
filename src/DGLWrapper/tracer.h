#include "DGLCommon/gl-types.h"
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>



class RetValue {
public: 
   RetValue():m_isSet(false) {}
   template<typename T>
   RetValue(T v):m_isSet(true), m_value(v) {}

   template<typename T>
   operator T() { return boost::get<T>(m_value); }

   bool isSet() { return m_isSet; }   

private: 
    bool m_isSet;

    typedef int (WINAPI *NativeEntrpType)(void);

    boost::variant<GLboolean, GLuint, GLint, const GLubyte*, NativeEntrpType, HDC, HGLRC> m_value;
};


class ITracer {
public: 
    virtual RetValue Pre(Entrypoint) = 0; 
    virtual void Post(Entrypoint) = 0; 
};


class DefaultTracer: public ITracer {
    virtual RetValue Pre(Entrypoint); 
    virtual void Post(Entrypoint call);
};



extern boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];

template<typename Tracer>
void SetAllTracers() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Tracers[i] = boost::shared_ptr<ITracer>(new Tracer());
    }
}


