#include <DGLNet/server.h>
#include <DGLNet/message.h>

#include "debugger.h"
#include "tracer.h"




boost::shared_ptr<ITracer> g_Tracers[NUM_ENTRYPOINTS];


RetValue DefaultTracer::Pre(Entrypoint entrp) {
   g_Server->lock();
    
    do {
        dglnet::CurrentCallStateMessage callStateMessage(entrp);
        g_Server->sendMessage(&callStateMessage);


        g_Server->poll();
    } while (g_BreakState.isBreaked());
    
    g_Server->unlock();
    return RetValue();
}

void DefaultTracer::Post(Entrypoint call) {}