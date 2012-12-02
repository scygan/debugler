#ifndef DGLCONTROLLER_H
#define DGLCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

#include "DGLNet/client.h"
#include "DGLCommon/dglconfiguration.h"
#include <boost/make_shared.hpp>

class DglController;

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
    virtual void doHandle(const dglnet::BreakedCallMessage&);
    virtual void doHandle(const dglnet::CallTraceMessage&);
    virtual void doHandle(const dglnet::TextureMessage&);
    virtual void doHandle(const dglnet::BufferMessage&);
    virtual void doHandle(const dglnet::FramebufferMessage&);
    virtual void doHandle(const dglnet::FBOMessage&);
    virtual void doHandle(const dglnet::ShaderMessage&);
    virtual void doHandle(const dglnet::ProgramMessage&);

    /** 
     * Method called by DGLclient, when disconnection condition is detected
     */
    virtual void doHandleDisconnect(const std::string&);

    /** 
     * Method called to request information on given texture from debugee
     */
    void requestTexture(uint name, bool focus = true);

    /** 
     * Method called to request information on given buffer from debugee
     */
    void requestBuffer(uint name, bool focus = true);

    /** 
     * Method called to request information on given frame buffer from debugee
     */
    void requestFramebuffer(uint bufferEnum, bool focus = true);

    /** 
     * Method called to request information on given frame buffer object from debugee
     */
    void requestFBO(uint name, bool focus = true);

     /** 
     * Method called to request information on given shader object from debugee
     */
    void requestShader(uint name, uint target, bool focus = true);

    /** 
     * Method called to request information on given shader program object from debugee
     */
    void requestProgram(uint name, bool focus = true);

    /**
     * Getter for break point controller object
     */
    DGLBreakPointController* getBreakPoints();

    /** 
     * Setter for new configuration of debugee
     */
    void configure(bool breakOnGLError);

    const DGLConfiguration& getConfig();

signals:
    void disconnected();
    void connected();

    void breaked(CalledEntryPoint, uint);
    void running();
    void breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&);

    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);
    void gotTexture(uint, const dglnet::TextureMessage&);
    void gotBuffer(uint, const dglnet::BufferMessage&);
    void gotFramebuffer(uint, const dglnet::FramebufferMessage&);
    void gotFBO(uint, const dglnet::FBOMessage&);
    void gotShader(uint, const dglnet::ShaderMessage&);
    void gotProgram(uint, const dglnet::ProgramMessage&);

    void newStatus(const QString&);
    void error(const QString&, const QString&);

    void focusTexture(uint name);
    void focusBuffer(uint name);
    void focusFramebuffer(uint bufferEnum);
    void focusFBO(uint name);
    void focusShader(uint name, uint target);
    void focusProgram(uint name);
    
public slots:
    void poll();
    void debugContinue();   
    void debugInterrupt();   
    void debugStep();
    void debugStepDrawCall();
    void debugStepFrame();
    void queryCallTrace(uint, uint);

private:
    void sendMessage(dglnet::Message*);

    boost::shared_ptr<dglnet::Client> m_DglClient;
    boost::shared_ptr<QSocketNotifier> m_NotifierRead, m_NotifierWrite;
    QTimer m_Timer;
    bool m_DglClientDead;
    std::string m_DglClientDeadInfo;
    DGLBreakPointController m_BreakPointController;
    DGLConfiguration m_Config;
};

#endif