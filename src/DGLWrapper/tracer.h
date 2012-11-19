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

class TracerBase {
public:
    virtual ~TracerBase() {}
    virtual RetValue Pre(const CalledEntryPoint&);
    virtual void Post(const CalledEntryPoint&, const RetValue& ret = RetValue());

    template<typename SpecificTracerType> 
    static void SetNext(Entrypoint entryp) {
        boost::shared_ptr<TracerBase> prev = g_Tracers[entryp];
        g_Tracers[entryp] = boost::shared_ptr<TracerBase>(new SpecificTracerType());
        g_Tracers[entryp]->SetPrev(prev);
    }
protected:
    RetValue PrevPre(const CalledEntryPoint&);
    void PrevPost(const CalledEntryPoint&, const RetValue& ret);
private:
    void SetPrev(const boost::shared_ptr<TracerBase>& prev);
    boost::shared_ptr<TracerBase> m_PrevTracer;
};

class DefaultTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
};

class GetProcAddressTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
};

class ContextTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class TextureTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class BufferTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ProgramTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class FBOTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};



extern boost::shared_ptr<TracerBase> g_Tracers[NUM_ENTRYPOINTS];

template<typename Tracer>
void SetAllTracers() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Tracers[i] = boost::shared_ptr<TracerBase>(new Tracer());
    }
}
