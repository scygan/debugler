#ifndef DGLCONTROLLER_H
#define DGLCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

#include "DGLNet/client.h"
#include <boost/make_shared.hpp>

class DglController: public QObject, public dglnet::IController, public dglnet::MessageHandler {
    Q_OBJECT

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
    virtual void doHandleDisconnect(const std::string&);

    //GUI interactions:

    void showTexture(uint name);

signals:
    void disconnected();
    void connected();

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

    void queryCallTrace(uint, uint);

private:
    boost::shared_ptr<dglnet::Client> m_DglClient;
    boost::shared_ptr<QSocketNotifier> m_NotifierRead, m_NotifierWrite;
    QTimer m_Timer;
    bool m_DglClientDead;
};

#endif