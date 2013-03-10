#include <DGLCommon/gl-serialized.h>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>
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

class TracerBase;
extern boost::shared_ptr<TracerBase> g_Tracers[NUM_ENTRYPOINTS];

class TracerBase {
public:
    virtual ~TracerBase() {}

    /** 
     * Entrypoint fo r Pre() tracing, called by wrappers
     */
    virtual RetValue DoPre(const CalledEntryPoint&);

    /** 
     * Entrypoint fo r Post() tracing, called by wrappers
     */
    virtual void DoPost(const CalledEntryPoint&, const RetValue& ret = RetValue());

    template<typename SpecificTracerType> 
    static void SetNext(Entrypoint entryp) {
        boost::shared_ptr<TracerBase> prev = g_Tracers[entryp];
        g_Tracers[entryp] = boost::shared_ptr<TracerBase>(new SpecificTracerType());
        g_Tracers[entryp]->SetPrev(prev);
    }
protected:
    /** 
     * Call previous tracer in Chain of Dependency
     */
    RetValue PrevPre(const CalledEntryPoint&);

    /** 
     * Call previous tracer in Chain of Dependency
     */
    void PrevPost(const CalledEntryPoint&, const RetValue& ret);

     /** 
     * Default, empty Pre() tracer. Subclasses may want to reimplement this
     */
    virtual RetValue Pre(const CalledEntryPoint&);

    /** 
     * Default, empty Post() tracer. Subclasses may want to reimplement this
     */
    virtual void Post(const CalledEntryPoint&, const RetValue& ret = RetValue());
private:
    static boost::thread_specific_ptr<int> m_ThreadedInfiniteRecursionGuard;

    void SetPrev(const boost::shared_ptr<TracerBase>& prev);
    boost::shared_ptr<TracerBase> m_PrevTracer;
};

class DefaultTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class GLGetErrorTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
};

class GetProcAddressTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
};

class ContextTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class DebugContextTracer: public TracerBase {
    virtual RetValue Pre(const CalledEntryPoint&); 
    static bool anyContextPresent;
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

class ShaderTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

class ImmediateModeTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};


class FBOTracer: public TracerBase {
    virtual void Post(const CalledEntryPoint&, const RetValue& ret);
};

template<typename Tracer>
void SetAllTracers() {
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        g_Tracers[i] = boost::shared_ptr<TracerBase>(new Tracer());
    }
}
