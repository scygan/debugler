#include <DGLNet/server.h>
#include <DGLNet/message.h>

#include "gl-wrappers.h"
#include "debugger.h"
#include "tracer.h"
#include "api-loader.h"


boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];


RetValue DefaultTracer::Pre(const CalledEntryPoint& call) {
    g_Controller->getServer().lock();

    //do a fast non-blocking poll to get "interrupt" message, etc.."
    g_Controller->getServer().poll();

    if (g_Controller->getBreakState().isBreaked()) {
        //we just hit a break;
        dglnet::BreakedCallMessage callStateMessage(call, g_Controller->getCallHistory().size());
        g_Controller->getServer().sendMessage(&callStateMessage);
    }
    
    while (g_Controller->getBreakState().isBreaked()) {
        //block & loop until someone unbreaks us
        g_Controller->getServer().run_one();
    }

    g_Controller->getCallHistory().add(call);

    g_Controller->getBreakState().endStep();
    g_Controller->getServer().unlock();
    return RetValue();
}

void DefaultTracer::Post(const CalledEntryPoint& call) {}

RetValue GetProcAddressTracer::Pre(const CalledEntryPoint& call) {
    RetValue ret = DefaultTracer::Pre(call);

    if (ret.isSet()) return ret;

    Entrypoint entryp;
    const char* entrpName; call.getArgs()[0].get(entrpName);
    try {
        entryp = GetEntryPointEnum(entrpName);
    } catch(const std::runtime_error&) {
        //we do not support this entrypoint
        //TODO: add partial support for unknown entrypoints
        return ret;  
    }
    //we recognize this entrypoint, load if nessesary and return address to  wrapper
    LoadOpenGLExtPointer(entryp);
    ret = getWrapperPointer(entryp);
    return ret;
}

void GetProcAddressTracer::Post(const CalledEntryPoint& call) {
    DefaultTracer::Post(call);
}
