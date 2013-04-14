#ifndef WA_H
#define WA_H

//WA list 

//Buggy ARM Mali OpenGL ES 3.0 emulator: really much data is returned from glGetIntegerv
//Fix: use larger buffer
#define WA_ARM_MALI_EMU_GETTERS_OVERFLOW

// Buggy ARM Mali OpenGL ES 3.0 emulator: EGL_CONFIG_ID queried using eglQuerySurface does not match any eglConfig
#define WA_ARM_MALI_EMU_EGL_QUERY_SURFACE_CONFIG_ID

 //Buggu ARM Mali OpenGL ES emulator on Windows: 
    //do not exit remote loader thread before app tear down
    //  Normally we would just return from this (empty) function causing loader
    //  thread to exit (leaving app in suspended state - no user threads). 
    //  This would also cause DLL_THREAD_DETACH on all recently loaded DLLs. Unfortunately
    //  this causes CreateWindowEx() to fail later in eglInitialize(), propably because
    //  RegisterClass was called in this thread (sic!) from DLLMain.
//Fix: lock this thread until application finishes
#define WA_ARM_MALI_EMU_LOADERTHREAD_KEEP


//Querying GL state is not implemented propely on ES. May be related to WA_ARM_MALI_EMU_GETTERS_OVERFLOW
//Fix: disable query on ES
#define WA_ES_QUERY_STATE


#endif //WA_H
