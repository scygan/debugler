#ifndef _SERVER_H
#define _SERVER_H

#include "DGLNet/transport.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>


namespace dglnet {

class Server: public Transport {
public: 
    Server(int port, MessageHandler*);
    void lock();
    void unlock();

protected:
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::mutex m_mutex;
};

}

#endif