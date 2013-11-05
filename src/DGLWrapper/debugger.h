/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <DGLNet/transport.h>
#include <boost/circular_buffer.hpp>

#include <DGLNet/protocol/dglconfiguration.h>
#include <DGLNet/protocol/request.h>
#include <DGLCommon/os.h>
#include <DGLCommon/ipc.h>

#include "gl-state.h"

#include <mutex>

/**
 * class for break state - breaking and continuing application execution
 * handles breakpoints and breaking on them (and other events)
 */
class BreakState {
public:

    /**
     * Ctor
     */
    BreakState(bool breaked);

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
    void handle(const dglnet::message::Configuration&);

    /**
     * Handler for remote continue & break commands
     */
    void handle(const dglnet::message::ContinueBreak&);

    /**
     * Handler for set breakpoints command
     */
    void handle(const dglnet::message::SetBreakPoints&);
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
    dglnet::message::ContinueBreak::StepMode m_StepMode;

     /** 
      * List of actually set breakpoints
      */
    std::set<Entrypoint> m_BreakPoints;
};

/**
 * Length of history buffer
 */
#define CALL_HISTORY_LEN 5000

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
    void query(const dglnet::message::QueryCallTrace& query, dglnet::message::CallTrace& reply);


    /** 
     * Getter for call history size
     */
    size_t size();

    /** 
     * Set return value of last GL call
     */
    void setRetVal(const RetValue& ret);

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
    std::mutex m_mutex;
};


class DGLDebugController;

/** 
 *  Server conenction holder
 *
 */
class DGLDebugServer {
public: 

    /**
     * Ctor
     */
    DGLDebugServer(DGLDebugController*);

    /** 
     * Getter for mutex guarding server
     */
    std::mutex& getMutex();


    std::shared_ptr<dglnet::ITransport> getTransport();

    /** 
     *  Constructs and sets server of given type
     */ 
    template<class server_type>
    void listen(const std::string& port, bool wait);

    /** 
     * Disconnect listening server
     */
    void abort();

private:

    /**
     * Server object
     */
    std::shared_ptr<dglnet::ITransport> m_Transport;

    /**
     * Mutex locking server object
     */
    std::mutex m_ServerMutex;


    /** 
     * Parrent object
     */
    DGLDebugController* m_parrent;
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
     * Handler of begin of listening event. 
     *
     * Sets status presenter
     */
    virtual void doHandleListen(const std::string& port);

    /** 
     * Handler of connection event. 
     *
     * Sends Hello packet
     */
    virtual void doHandleConnect();

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
    DGLDebugServer& getServer();

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
    void doHandleConfiguration(const dglnet::message::Configuration&) override;

    /** 
     * Message handler - pass continue & break message to breakstate object
     */
    void doHandleContinueBreak(const dglnet::message::ContinueBreak&) override;

    /** 
     * Message handler - pass history query message to call history object
     */
    void doHandleQueryCallTrace(const dglnet::message::QueryCallTrace&) override;

    /** 
     * Message handler - pass new breakpoint list message to breakstate object
     */
    void doHandleSetBreakPoints(const dglnet::message::SetBreakPoints&) override;

    /** 
     * Message handler - handle general request message
     */
    void doHandleRequest(const dglnet::message::Request&) override;

    /** 
     * Request handler - query resource request
     */
    boost::shared_ptr<dglnet::DGLResource> doHandleRequest(const dglnet::request::QueryResource&);

    /** 
     * Request handler - edit shader request
     */
    void doHandleRequest(const dglnet::request::EditShaderSource&);

    /** 
     * Request handler - link program request
     */
    void doHandleRequest(const dglnet::request::ForceLinkProgram&);

    /**
     *  Abnormal termination exception class
     *
     *  thrown by poll() and run_one
     */
    class TeardownException {};
private:

    /** 
     * Call history object
     */
    CallHistory m_CallHistory;

    /**
     * Status presenter (baloon presenter)
     */
    std::shared_ptr<OsStatusPresenter> m_presenter;

    /**
     * True if served notified disconnection and it's destruction is pending
     */
    bool m_Disconnected;

    /**
     * Application abnormal terminator
     */
    void tearDown();


    /**
     * Server connection
     */
    DGLDebugServer m_Server;

};

/** 
 *  Global controller object instance
 */
extern std::shared_ptr<DGLDebugController> _g_Controller;

/** 
 *  Global controller object instance getter
 */
DGLDebugController* getController();

/** 
 *  Global config object instance
 */
extern DGLConfiguration g_Config;

/**
 * Get IPC handle to loader 
 */
DGLIPC* getIPC();


#endif //DEBUGGER_H
