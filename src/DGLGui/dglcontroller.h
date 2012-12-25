#ifndef DGLCONTROLLER_H
#define DGLCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

#include "DGLNet/client.h"
#include "DGLCommon/dglconfiguration.h"
#include <boost/make_shared.hpp>

class DGLResourceManager;

class DGLResourceListener: public QObject {
    friend class DGLResourceManager;
public:
    Q_OBJECT

    DGLResourceListener(uint listenerId, uint objectId, DGLResource::ObjectType type, DGLResourceManager* manager);

    ~DGLResourceListener();

signals:
    void update(const DGLResource&);
    void error(const std::string&);
    void invalidate();
private:
    uint m_ListenerId, m_ObjectId;
    DGLResourceManager* m_Manager;
    DGLResource::ObjectType m_ObjectType;
    uint m_RefCount;
};


class DglController;

class DGLResourceManager {
    friend class DGLResourceListener;
public:
    DGLResourceManager(DglController*);


    void emitQueries();

    void handleResourceMessage(const dglnet::ResourceMessage& msg);

    DGLResourceListener* createListener(uint objectId, DGLResource::ObjectType type);

private:
    void registerListener(DGLResourceListener* listener);

    void unregisterListener(DGLResourceListener* listener);

    std::multimap<uint, DGLResourceListener*> m_Listeners;

    uint m_MaxListenerId;

    DglController* m_Controller;
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


class DGLViewRouter:public QObject {
    Q_OBJECT
public:
    void show(uint name, DGLResource::ObjectType type, uint target = 0);
signals:
    void showTexture(uint name);
    void showBuffer(uint name);
    void showFramebuffer(uint bufferEnum);
    void showFBO(uint name);
    void showShader(uint name, uint target);
    void showProgram(uint name);
};

/** 
 * DGLController class - the interface between UI and debugee
 */
class DglController: public QObject, public dglnet::IController, public dglnet::MessageHandler {
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
    

    //IController methods:
    /**
     * Method called by DGLClient on connection state change
     */
    virtual void onSetStatus(std::string);

    /**
     * Method called by DGLClient, when network socket is created
     */
    virtual void onSocket();

    //IMessageHandler methods:
    virtual void doHandle(const dglnet::HelloMessage&);
    virtual void doHandle(const dglnet::BreakedCallMessage&);
    virtual void doHandle(const dglnet::CallTraceMessage&);
    virtual void doHandle(const dglnet::ResourceMessage&);

    /** 
     * Method called by DGLclient, when disconnection condition is detected
     */
    virtual void doHandleDisconnect(const std::string&);

    /**
     * Getter for break point controller object
     */
    DGLBreakPointController* getBreakPoints();

    /**
     * Getter for resource manager controller object
     */
    DGLResourceManager* getResourceManager();

     /**
     * Getter for view router controller object
     */
    DGLViewRouter* getViewRouter();

    /** 
     * Setter for new configuration of debugee
     */
    void configure(bool breakOnGLError, bool breakOnDebugOutput, bool breakOnCompilerError);


    const DGLConfiguration& getConfig();

    /** 
     * Send message to debugee. Base method for all lover-level communication with debugee
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
    void setDisconnected(bool);

    /** 
     * Signal for setting GUI state
     */
    void setBreaked(bool);

    /** 
     * Signal for setting GUI state
     */
    void setRunning(bool);

    void breaked(CalledEntryPoint, uint);
    void breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&);

    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);

    void newStatus(const QString&);
    void error(const QString&, const QString&);
  
public slots:
    void poll();
    void debugContinue();   
    void debugInterrupt();   
    void debugStep();
    void debugStepDrawCall();
    void debugStepFrame();
    void queryCallTrace(uint, uint);

private:

    void sendConfig();

    boost::shared_ptr<dglnet::Client> m_DglClient;
    boost::shared_ptr<QSocketNotifier> m_NotifierRead, m_NotifierWrite;
    QTimer m_Timer;

    /**
     * Set to true, if client is no longer connected (setting to true from asio handler will cause disconnection)
     */
    bool m_Disconnected;

    /**
     * Set to true if configuration and breakpoints were already send.
     * If not set, the configuration and breakpoint will be synced ASAP.
     */
    bool m_ConfiguredAndBkpointsSet;

    /**
     * Set to true, if client is ready for receiving messages. Does not always equal !m_Disconnected
     */
    bool m_Connected;

    std::string m_DglClientDeadInfo;
    DGLBreakPointController m_BreakPointController;
    DGLConfiguration m_Config;
    DGLResourceManager m_ResourceManager;
    DGLViewRouter m_ViewRouter;
};

#endif