#include "server.h"


namespace dglnet {

    Server::Server(int port, MessageHandler* handler):Transport(handler),m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(m_io_service, m_endpoint) {}

    void Server::accept() {
        m_acceptor.listen();
        boost::system::error_code ec;
        m_acceptor.accept(m_socket, ec);
        if (ec) {
            notifyDisconnect(ec.message());
        } else {
            read();
        }
    }

    void Server::lock() {
        m_mutex.lock();
    }    

    void Server::unlock() {
        m_mutex.unlock();
    }  
}

