#include <DGLNet/server.h>
#include <DGLNet/message.h>

#include "debugger.h"
#include "tracer.h"




boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];


RetValue DefaultTracer::Pre(Entrypoint entrp) {
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

