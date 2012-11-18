#ifndef DGLCONTROLLER_H
#define DGLCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

#include "DGLNet/client.h"
#include <boost/make_shared.hpp>

class DglController;

class DGLBreakPointController {
public:
    DGLBreakPointController(DglController* controller);
    std::set<Entrypoint> getCurrent();
    void setCurrent(const std::set<Entrypoint>&);
private:
    std::set<Entrypoint> m_Current;
    DglController* m_Controller;
};

class DglController: public QObject, public dglnet::IController, public dglnet::MessageHandler {
    Q_OBJECT

friend class DGLBreakPointController;

public:
    DglController();
    void connectServer(const std::string& host, const std::string& port);
    void disconnectServer();

    //IController methods:
    virtual void onSetStatus(std::string);
    virtual void onSocket();

    //IMessageHandler methods:
    virtual void doHandle(const dglnet::BreakedCallMessage&);
    virtual void doHandle(const dglnet::CallTraceMessage&);
    virtual void doHandle(const dglnet::TextureMessage&);
    virtual void doHandle(const dglnet::BufferMessage&);
    virtual void doHandle(const dglnet::FramebufferMessage&);
    virtual void doHandleDisconnect(const std::string&);

    //GUI interactions
    void doShowTexture(uint name);
    void doShowBuffer(uint name);
    void doShowFramebuffer(uint bufferEnum);
    DGLBreakPointController* getBreakPoints();

signals:
    void disconnected();
    void connected();

    void breaked(CalledEntryPoint, uint);
    void breakedWithStateReports(uint, const std::vector<dglnet::ContextReport>&);

    void gotCallTraceChunkChunk(uint, const std::vector<CalledEntryPoint>&);
    void gotTexture(uint, const dglnet::TextureMessage&);
    void gotBuffer(uint, const dglnet::BufferMessage&);
    void gotFramebuffer(uint, const dglnet::FramebufferMessage&);

    void newStatus(const QString&);
    void error(const QString&, const QString&);

    void showTexture(uint name);
    void showBuffer(uint name);
    void showFramebuffer(uint bufferEnum);
    
    
public slots:
    void poll();
    void debugContinue();   
    void debugInterrupt();   
    void debugStep();

    void debugQueryTexture(uint);
    void debugQueryBuffer(uint);
    void debugQueryFramebuffer(uint);

    void queryCallTrace(uint, uint);

private:
    void sendMessage(dglnet::Message*);

    boost::shared_ptr<dglnet::Client> m_DglClient;
    boost::shared_ptr<QSocketNotifier> m_NotifierRead, m_NotifierWrite;
    QTimer m_Timer;
    bool m_DglClientDead;
    DGLBreakPointController m_BreakPointController;
};

#endif