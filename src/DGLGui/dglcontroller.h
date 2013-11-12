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

#ifndef DGLCONTROLLER_H
#define DGLCONTROLLER_H

#include "dglqtgui.h"
#include <QTimer>
#include <QSocketNotifier>

#include <DGLNet/client.h>
#include <DGLNet/protocol/dglconfiguration.h>
#include <DGLNet/protocol/resource.h>
#include <DGLNet/protocol/request.h>
#include <boost/make_shared.hpp>

#include <DGLCommon/def.h>

class DGLRequestManager;

/**
 * Class implementing handler for response for single-time request
 *
 * Requests are all messages send to debugee, that can emit errors upon receive
 *(so reply is needed).
 */
class DGLRequestHandler {
   public:
    DGLRequestHandler(DGLRequestManager*);
    virtual void onRequestFinished(
        const dglnet::message::RequestReply* reply) = 0;
    virtual ~DGLRequestHandler();

   private:
    DGLRequestManager* m_Manager;
};

class DglController;

/**
 * Class managing and handling all issued and not replied requests
 */
class DGLRequestManager {
   public:
    DGLRequestManager(DglController*);

    void request(dglnet::DGLRequest* request, DGLRequestHandler*);

    void handle(const dglnet::message::RequestReply& msg);

    void unregisterHandler(DGLRequestHandler*);

   private:
    DglController* m_Controller;

    std::map<int, DGLRequestHandler*> m_CurrentHandlers;
};

class DGLResourceManager;

/**
 * Class implementint a resource listener
 *
 * Each listener is associated with one resource to be queried multiple times
 * Queries are issues automatically by DGLResourceManager on DGLController
 *command.
 */
class DGLResourceListener : public QObject, public DGLRequestHandler {
    friend class DGLResourceManager;

   public:
    Q_OBJECT

    DGLResourceListener(dglnet::ContextObjectName objectName,
                        dglnet::DGLResource::ObjectType type,
                        DGLResourceManager* manager);

    ~DGLResourceListener();

    virtual void onRequestFinished(const dglnet::message::RequestReply* msg);

   public:
    void fire();

signals:
    void update(const dglnet::DGLResource&);
    void error(const std::string&);

   private:
    dglnet::DGLResource::ObjectType m_ObjectType;
    dglnet::ContextObjectName m_ObjectName;
    DGLResourceManager* m_Manager;
};

/**
 * Class managing and handling all listeners
 *
 * It's main aim is to emit apropriate queries and return queried data to
 *listeners
 */
class DGLResourceManager {
    friend class DGLResourceListener;

   public:
    DGLResourceManager(DGLRequestManager*);

    void emitQueries();

    DGLResourceListener* createListener(dglnet::ContextObjectName name,
                                        dglnet::DGLResource::ObjectType type);

    DGLRequestManager* getRequestManager();

   private:
    void unregisterListener(DGLResourceListener* listener);

    std::list<DGLResourceListener*> m_Listeners;

    DGLRequestManager* m_RequestManager;
};

/**
 * Class aggregating currently set bkpoints
 */
class DGLBreakPointController {
   public:
    /**
     * CTor
     * @param DGLController object, for communation with debugee
     */
    DGLBreakPointController(DglController* controller);

    /**
     * Getter for list of current bkpoints
     */
    std::set<Entrypoint> getCurrent();

    /**
     * Set current entrypoints and send them to debugee
     */
    void setCurrent(const std::set<Entrypoint>&);

    /**
     * Send breakpoints to debugee
     */
    void sendCurrent();

   private:
    /**
     * List of currently set breakpoints
     */
    std::set<Entrypoint> m_Current;

    /**
     * DGLController object, which we send new breakpoint lists to
     */
    DglController* m_Controller;
};

class DGLViewRouter : public QObject {
    Q_OBJECT
   public:
    void show(const dglnet::ContextObjectName& name,
              dglnet::DGLResource::ObjectType type);
signals:
    void showTexture(opaque_id_t ctx, gl_t name);
    void showBuffer(opaque_id_t ctx, gl_t name);
    void showFramebuffer(opaque_id_t ctx, gl_t bufferEnum);
    void showFBO(opaque_id_t ctx, gl_t name);
    void showShader(opaque_id_t ctx, gl_t name, gl_t target);
    void showProgram(opaque_id_t ctx, gl_t name);
};

/**
 * DGLController class - the interface between UI and debugee
 */
class DglController : public QObject,
                      public dglnet::IController,
                      public dglnet::MessageHandler {
    Q_OBJECT

    friend class DGLBreakPointController;

   public:
    DglController();

    /**
     * Start a new network connection to already running debugee
     * This can be also known as "attaching"
     */
    void connectServer(const std::string& host, const std::string& port);

    /**
     * Terminate ongoing connection to debugee
     * The debugee should terminate as a side effect of disconnection
     */
    void disconnectServer();

    /**
     * Getter for checking if we are connected to debugee
     */
    bool isConnected();

    // IController methods:
    /**
     * Method called by DGLClient on connection state change
     */
    virtual void onSetStatus(std::string);

    /**
     * Method called by DGLClient, when network socket is created
     */
    virtual void onSocket();

    /**
     * Method called by DGLClient, when network socket starts sending data
     *
     * We start QSocketNotifier in write mode to handle polls()
     */
    virtual void onSocketStartSend();

    /**
     * Method called by DGLClient, when network socket stops sending data
     *
     * We stop QSocketNotifier in write mode to stop handling polls()
     */
    virtual void onSocketStopSend();

    // IMessageHandler methods:
    virtual void doHandleHello(const dglnet::message::Hello&) override;
    virtual void doHandleBreakedCall(const dglnet::message::BreakedCall&)
        override;
    virtual void doHandleCallTrace(const dglnet::message::CallTrace&) override;
    virtual void doHandleRequestReply(const dglnet::message::RequestReply&)
        override;

    /**
     * Method called by DGLclient, upon successful connection established
     */
    virtual void doHandleConnect();

    /**
     * Method called by DGLclient, when disconnection condition is detected
     */
    virtual void doHandleDisconnect(const std::string&) override;

    /**
     * Getter for break point controller object
     */
    DGLBreakPointController* getBreakPoints();

    /**
     * Getter for request manager controller object
     */
    DGLRequestManager* getRequestManager();

    /**
     * Getter for resource manager controller object
     */
    DGLResourceManager* getResourceManager();

    /**
    * Getter for view router controller object
    */
    DGLViewRouter* getViewRouter();

    /**
     * Send new configuration to debugee
     */
    void sendConfig(const DGLConfiguration* config = NULL);

    DGLConfiguration& getConfig();

    /**
     * Send message to debugee. Base method for all lover-level communication
     * with debugee
     */
    void sendMessage(dglnet::Message*);

signals:

    /**
    * Signal for setting GUI state
    */
    void debugeeInfo(const std::string&);

    /**
    * Signal for setting GUI state
    */
    void setConnected(bool);

    /**
     * Signal for setting GUI state
     */
    void setBreaked(bool);

    /**
     * Signal for setting GUI state
     */
    void setRunning(bool);

    void breaked(CalledEntryPoint, uint);
    void breakedWithStateReports(
        opaque_id_t,
        const std::vector<dglnet::message::BreakedCall::ContextReport>&);

    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);

    void newStatus(const QString&);
    void connectionLost(const QString&, const QString&);

   public
slots:
    void poll();
    void debugContinue();
    void debugInterrupt();
    void debugStep();
    void debugStepDrawCall();
    void debugStepFrame();
    void queryCallTrace(uint, uint);

   private:
    std::shared_ptr<dglnet::Client> m_DglClient;
    std::shared_ptr<QSocketNotifier> m_NotifierRead, m_NotifierWrite;
    QTimer m_Timer;

    /**
     * Set to true, if client is no longer connected (setting to true from asio
     * handler will cause disconnection)
     */
    bool m_Disconnected;

    /**
     * Set to true, if client is ready for receiving messages. Does not always
     * equal !m_Disconnected
     */
    bool m_Connected;

    /**
     * Set to true if configuration and breakpoints were already send.
     * If not set, the configuration and breakpoint will be synced ASAP.
     */
    bool m_ConfiguredAndBkpointsSet;

    std::string m_DglClientDeadInfo;
    DGLBreakPointController m_BreakPointController;
    DGLConfiguration m_Config;
    DGLRequestManager m_RequestManager;
    DGLResourceManager m_ResourceManager;
    DGLViewRouter m_ViewRouter;
};

#endif
