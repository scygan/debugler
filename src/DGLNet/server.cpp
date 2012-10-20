#include "server.h"


namespace dglnet {

    Server::Server(int port):m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(m_io_service, m_endpoint) {
        m_acceptor.listen();
        boost::system::error_code err;
        m_acceptor.accept(m_socket, err);
    }

    void Server::lock() {
        m_mutex.lock();
    }    
    void Server::unlock() {
        m_mutex.lock();
    }  
}

