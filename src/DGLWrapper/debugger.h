#include <DGLNet/server.h>

#include<map>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <DGLCommon/dglconfiguration.h>

#include "gl-state.h"


/**
 * Length of history buffer
 */
#define CALL_HISTORY_LEN 5000

/**
 * Class aggregating all OpenGL state read needed by debugger
 */
class DGLGLState {

    /**
     * Iterator for container of all GL context state objects
     */
    typedef std::map<uint32_t, boost::shared_ptr<dglState::GLContext> >::iterator ContextListIter;

    /**
     * Iterator for container of all native surfaces
     */
    typedef std::map<uint32_t, boost::shared_ptr<dglState::NativeSurface> >::iterator SurfaceListIter;
public:

    /**
     * Ctor
     */
    DGLGLState();

    /**
     * Getter for current context state object
     */
    dglState::GLContext* getCurrent();

    /**
     * Getter for ctx object by given id (created if not exist)
     */
    ContextListIter ensureContext(uint32_t id, bool lock = true);

    /**
     * Getter for native surface object by given id (created if not exist)
     */
    SurfaceListIter ensureSurface(uint32_t id, bool lock = true);

    /**
     * Setter for current context (should be called just after *MakeCurrent-like calls).
     */
    void bindContext(uint32_t id, uint32_t hdc);

    /**
     * Method deleting ctx object by given id (should be called when deleted by application (may be still used)
     */
    void deleteContext(uint32_t id);

    /**
     * Getter for shourt context state report
     */
    std::vector<dglnet::ContextReport> describe();

private:

    /**
     * Container of all GL context state objects
     */
    std::map<uint32_t, boost::shared_ptr<dglState::GLContext> > m_ContextList;

    /**
     * Container of  all native surfaces
     */
    std::map<uint32_t, boost::shared_ptr<dglState::NativeSurface> > m_SurfaceList;

    /**
     * Thread-space pointer to current context object
     */
    boost::thread_specific_ptr<dglState::GLContext> m_Current;

    /**
     * Thread-space pointer to native surface object
     */
    boost::thread_specific_ptr<dglState::NativeSurface> m_CurrentSurface;

    /**
     * Mutex for context container operations
     */
    boost::mutex m_ContextListMutex;

    /**
     * Mutex for surface container operations
     */
    boost::mutex m_SurfaceListMutex;
};


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
     * Dtor
     */
    ~DGLDebugController();

    /** 
     * Called when new connection is made in initialization process
     *
     * @arg1 newly conencted server object
     */
    void connect(boost::shared_ptr<dglnet::Server>);

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
 *  Global state object instance
 */
extern DGLGLState g_DGLGLState; 

/** 
 *  Global config object instance
 */
extern DGLConfiguration g_Config;