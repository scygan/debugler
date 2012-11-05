#include <DGLCommon/gl-serialized.h>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <vector>




class RetValue: public AnyValue {
public: 
   RetValue():m_isSet(false) {}

   RetValue(const RetValue& v):AnyValue(*static_cast<const AnyValue*>(&v)),m_isSet(v.isSet()) {}

   template<typename T>
   RetValue(T v):AnyValue(v),m_isSet(true) {}

   bool isSet() const { return m_isSet; }   
private: 
    bool m_isSet;
};



class ITracer {
public: 
    virtual RetValue Pre(const CalledEntryPoint&) = 0; 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret = RetValue()) = 0; 
};

class DefaultTracer: public ITracer {
protected:
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class GetProcAddressTracer: public DefaultTracer {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ContextTracer: public DefaultTracer {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class TextureTracer: public DefaultTracer {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class BufferTracer: public DefaultTracer {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ProgramTracer: public DefaultTracer {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};


extern boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];

template<typename Tracer>
void SetAllTracers() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Tracers[i] = boost::shared_ptr<ITracer>(new Tracer());
    }
}

template<typename Tracer>
void SetTracer(Entrypoint entryp) {
    g_Tracers[entryp] = boost::shared_ptr<ITracer>(new Tracer());
}


