#include <DGLNet/server.h>

#include<map>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <DGLCommon/dglconfiguration.h>
#include <DGLCommon/os.h>

#include "gl-state.h"


/**
 * Length of history buffer
 */
#define CALL_HISTORY_LEN 5000

/**
 * Class aggregating all OpenGL state read needed by debugger
 */
class DGLDisplayState {

    /**
     * Iterator for container of all GL context state objects
     */
    typedef std::map<uint32_t, boost::shared_ptr<dglState::GLContext> >::iterator ContextListIter;

    /**
     * Iterator for container of all native surfaces
     */
    typedef std::map<uint32_t, boost::shared_ptr<dglState::NativeSurfaceBase> >::iterator SurfaceListIter;
public:
    /**
     * Getter for default display on display-less configurations (like WGL).
     */
    static DGLDisplayState* default();

    /**
     * Getter for specific display on display-able configurations (like EGL, GLX).
     */
    static DGLDisplayState* get(uint32_t dpy);

    /**
     * Getter for ctx object by given id (created if not exist)
     */
    ContextListIter ensureContext(uint32_t id, bool lock = true);

    /**
     * Getter for native surface object by given id (created if not exist)
     */
    template<typename NativeSurfaceType>
    SurfaceListIter ensureSurface(uint32_t id, bool lock = true);

    /**
     * Method deleting ctx object by given id (should be called when deleted by application)
     */
    void deleteContext(uint32_t id);

    /**
     * Method deleting ctx object by given id (should be called when deleted by application). Does not immediately delete when bound
     */
    void lazyDeleteContext(uint32_t id);

    /**
     * Getter for short context state report
     */
    std::vector<dglnet::ContextReport> describe();

    /**
     * Getter for short context state report from all Displays
     */
    static std::vector<dglnet::ContextReport> describeAll();

private:

    /**
     * Container of all GL context state objects
     */
    std::map<uint32_t, boost::shared_ptr<dglState::GLContext> > m_ContextList;

    /**
     * Container of  all native surfaces
     */
    std::map<uint32_t, boost::shared_ptr<dglState::NativeSurfaceBase> > m_SurfaceList;

    /**
     * Mutex for context container operations
     */
    boost::mutex m_ContextListMutex;

    /**
     * Mutex for surface container operations
     */
    boost::mutex m_SurfaceListMutex;

    /**
     *  Collection of all displays
     */
    static std::map<uint32_t, boost::shared_ptr<DGLDisplayState> > s_Displays;

    /**
     *  Mutex guarding s_Displays
     */ 
    static boost::mutex s_DisplaysMutex;
};


class DGLThreadState;



/**
* Class aggregating all thread-specific state
*/
class DGLThreadState {
public:

    /**
     * Ctor
     */
    DGLThreadState();

    /**
     * Setter for current context (should be called just after *MakeCurrent-like calls).
     */
    void bindContext(DGLDisplayState* dpy, uint32_t id, dglState::NativeSurfaceBase* readSurface);

    /**
     * Getter for current context (should be called just after *MakeCurrent-like calls).
     */
    dglState::GLContext* getCurrentCtx();

    /**
     *  Release current TSS - should be called on eglReleaseThread
     */
    static void release();

     /**
     *  Get current TSS
     */
    static DGLThreadState* get();

    /**
     *  Set current API (only on EGL, called on eglBindApi)
     */
    void bindEGLApi(EGLenum api);

    /**
     *  Set current EGL API
     */
    EGLenum getEGLApi();

private:

    /**
     * Thread-space pointer to current context object
     */
    dglState::GLContext* m_Current;

    /**
     * Current EGL api
     */
    EGLenum m_EGLApi;

    //TODO: API bound by eglBindApi should be stored here

    /**
     * Thread-specific pointer to thread specific state
     */
    static boost::thread_specific_ptr<DGLThreadState> s_CurrentThreadState;
};

#define gc DGLThreadState::get()->getCurrentCtx()

/**
 * class for break state - breaking and continuing application execution
 * handles breakpoints and breaking on them (and other events)
 */
class BreakState {
public:

    /**
     * Ctor
     */
    BreakState();

    /**
     * Method deciding if application should be breaked at given entrypoint
     * this method considers earlier set breaks and breakpoint list
     *
     * @return true if application is to be breaked, false otherwise
     */
    bool mayBreakAt(const Entrypoint&);

    /**
     * Function set break state depending on given GL error
     * this method considers current debugger configuration before breaking app
     */
    void setBreakAtGLError(GLenum glError);

    /**
     * Function set break state, called when deboug output event was detected
     * this method considers current debugger configuration before breaking app
     */
    void setBreakAtDebugOutput();

    /**
     * Function set break state, called when compiler/linker error was detected
     * this method considers current debugger configuration before breaking app
     */
    void setBreakAtCompilerError();

    /**
     * Query breaked state
     * @return true if application is breaked, false otherwise
     */
    bool isBreaked();


    
    /**
     * Handler for configuration message remote command
     */
    void handle(const dglnet::ConfigurationMessage&);

    /**
     * Handler for remote continue & break commands
     */
    void handle(const dglnet::ContinueBreakMessage&);

    /**
     * Handler for set breakpoints command
     */
    void handle(const dglnet::SetBreakPointsMessage&);
private:

    /**
     * application break state (true if breaked, false otherwise)
     */
    bool m_break;

    /**
     * True if in step mode (pending break, despite m_break == false)
     * irrelevant, if m_break == true
     */
    bool m_StepModeEnabled;

    /** 
     * Actual step mode (call, draw call, frame) if in step mode
     * irrelevant, if m_StepModeEnabled == false
     */
    dglnet::ContinueBreakMessage::StepMode m_StepMode;

     /** 
      * List of actually set breakpoints
      */
    std::set<Entrypoint> m_BreakPoints;
};

/**
 * round buffer with call history
 */
class CallHistory {
public:
    /**
     * Ctor
     */
    CallHistory();

    /**
     * Add new history element to buffer
     */
    void add(const CalledEntryPoint&);

    /**
     * Handle and respond to history query message
     */  
    void query(const dglnet::QueryCallTraceMessage& query, dglnet::CallTraceMessage& reply);


    /** 
     * Getter for call history size
     */
    size_t size();

    /** 
     * set GL error on last called entrypoint
     */
    void setError(GLenum error);

    /** 
     * set debug output on last call entrypoint
     */
    void setDebugOutput(const std::string& message);
private:
    /** 
     * Actual round buffer with called entrypoints
     */
    boost::circular_buffer<CalledEntryPoint> m_cb;

    /** 
     * Mutex used to access this class
     */
    boost::mutex m_mutex;
};


/**
 * Master, wrapper-side debug controller
 *
 * This is the class that talks tu gui and does all of the debugging
 */
class DGLDebugController: public dglnet::MessageHandler {
public:

    /**
     * Ctor
     */
    DGLDebugController();

    /**
     * Dtor
     */
    ~DGLDebugController();

    /** 
     * Handler of disconnection
     * 
     * disconnection is always fatal - we should not continue apllication without gui. 
     * this function does not return.
     */
    virtual void doHandleDisconnect(const std::string&);

    /** 
     * Object handling breaks and entrypoints
     */
    BreakState m_BreakState;

    /** 
     * Getter for connected server object
     */
    dglnet::Server& getServer();

    /** 
     * Run one event on associated server;
     */
    void run_one();
    
    /** 
     * Poll events on associated server;
     */
    void poll();


    /** 
     * Getter for break state object
     */
    BreakState& getBreakState();

    /** 
     * Getter for break call history object
     */
    CallHistory& getCallHistory();

    /** 
     * Message handler - pass configuration message to global configuration object
     */
    void doHandle(const dglnet::ConfigurationMessage&);

    /** 
     * Message handler - pass continue & break message to breakstate object
     */
    void doHandle(const dglnet::ContinueBreakMessage&);

    /** 
     * Message handler - pass history query message to call history object
     */
    void doHandle(const dglnet::QueryCallTraceMessage&);

    /** 
     * Message handler - handle resource query
     */
    void doHandle(const dglnet::QueryResourceMessage&);

    /** 
     * Message handler - pass new breakpoint list message to breakstate object
     */
    void doHandle(const dglnet::SetBreakPointsMessage&);

private:

    /**
     * Server object
     */
    boost::shared_ptr<dglnet::Server> m_Server;

    /** 
     * Call history object
     */
    CallHistory m_CallHistory;

    /**
     * Status presenter (baloon presenter)
     */
    boost::shared_ptr<OsStatusPresenter> m_presenter;

    /**
     * True if served notified disconnection and it's destruction is pending
     */
    bool m_Disconnected;

    /**
     * Application abnormal terminator
     */
    void tearDown();

};

/** 
 *  Global controller object instance
 */
extern boost::shared_ptr<DGLDebugController> _g_Controller;

/** 
 *  Global controller object instance getter
 */
DGLDebugController* getController();

/** 
 *  Global config object instance
 */
extern DGLConfiguration g_Config;