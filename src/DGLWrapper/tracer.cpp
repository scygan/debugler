#include <DGLNet/server.h>
#include <DGLNet/message.h>

#include "gl-wrappers.h"
#include "debugger.h"
#include "tracer.h"
#include "api-loader.h"


boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];


RetValue DefaultTracer::Pre(Entrypoint entrp, const std::vector<AnyValue>&) {
    g_Controller->getServer().lock();

    //do a fast non-blocking poll to get "interrupt" message, etc.."
    g_Controller->getServer().poll();

    if (g_Controller->getBreakState().isBreaked()) {
        //we just hit a break;
        dglnet::BreakedCallMessage callStateMessage(entrp);
        g_Controller->getServer().sendMessage(&callStateMessage);
    }
    
    while (g_Controller->getBreakState().isBreaked()) {
        //block & loop until someone unbreaks us
        g_Controller->getServer().run_one();
    }

    g_Controller->getBreakState().endStep();
    g_Controller->getServer().unlock();
    return RetValue();
}

void DefaultTracer::Post(Entrypoint call) {}

RetValue GetProcAddressTracer::Pre(Entrypoint call, const std::vector<AnyValue>& args) {
    RetValue ret = DefaultTracer::Pre(call, args);

    if (ret.isSet()) return ret;

    Entrypoint entrp;
    try {
        entrp = GetEntryPointEnum((char*)(void*)args[0]);
    } catch(const std::runtime_error&) {
        //we do not support this entrypoint
        //TODO: add partial support for unknown entrypoints
        return ret;  
    }
    //we recognize this entrypoint, load if nessesary and return address to  wrapper
    LoadOpenGLExtPointer(entrp);
    ret = getWrapperPointer(entrp);
    return ret;
}

void GetProcAddressTracer::Post(Entrypoint call) {
    DefaultTracer::Post(call);
}
