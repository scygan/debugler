#include "server.h"

#include <boost/bind.hpp>

namespace dglnet {

    Server::Server(int port, MessageHandler* handler):Transport(handler),m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(m_io_service) {
        m_acceptor.open(m_endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(m_endpoint);
        m_acceptor.listen();
    }
    void Server::accept() {
        boost::system::error_code ec;
        ec = m_acceptor.accept(m_socket, ec);
        if (ec) {
            notifyDisconnect(ec.message());
        }
        read();
    }

    void Server::lock() {
        m_mutex.lock();
    }    

    void Server::unlock() {
        m_mutex.unlock();
    }  
}

