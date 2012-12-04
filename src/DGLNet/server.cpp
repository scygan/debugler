#include "server.h"

#include <boost/bind.hpp>

namespace dglnet {

    Server::Server(int port, MessageHandler* handler):Transport(handler),m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(m_io_service), m_Accepted(false) {
        m_acceptor.open(m_endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(m_endpoint);
        m_acceptor.listen();
    }

    void Server::startAccept() {
        m_acceptor.async_accept(m_socket, boost::bind(&Server::onAccept, shared_from_this(),
            boost::asio::placeholders::error));
        while (m_io_service.poll_one()) {}
    }

    void Server::onAccept(const boost::system::error_code &ec) {
        m_Accepted = true;
        if (ec) {
            notifyDisconnect(ec.message());
        }
    }

    void Server::waitAcceptedAndRead() {
        while (!m_Accepted) {
            m_io_service.run_one();
        }
        read();
    }

    void Server::lock() {
        m_mutex.lock();
    }    

    void Server::unlock() {
        m_mutex.unlock();
    }  

    boost::shared_ptr<Server> Server::shared_from_this() {
        return boost::static_pointer_cast<Server>(get_shared_from_base());
    }
}

