#include<string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "client.h"


namespace dglnet {

    Client::Client(IController* controller, MessageHandler* handler):Transport(handler),m_controller(controller), m_Resolver(m_io_service) {}

    void Client::connectServer(std::string host, std::string port) {
        boost::asio::ip::tcp::resolver::query query(host, port);
        m_Resolver.async_resolve(query, boost::bind(&Client::onResolve, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
        m_controller->onSetStatus("Looking up server...");
    }

    Client::socket_fd_t Client::getSocketFD() {
        return m_socket.native_handle();
    }

    void Client::onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
        if (!err) {
            m_socket.async_connect(*endpoint_iterator, boost::bind(&Client::onConnect, shared_from_this(),
                boost::asio::placeholders::error));
            m_controller->onSetStatus("Connecting...");
            m_controller->onSocket();
        } else {
            notifyDisconnect(err.message());
        }
    }

    void Client::onConnect(const boost::system::error_code &err) {
        if (!err) {
            m_controller->onSetStatus("Connected.");
            read();
        } else {
            notifyDisconnect(err.message());
        }
    }

    boost::shared_ptr<Client> Client::shared_from_this() {
        return boost::static_pointer_cast<Client>(get_shared_from_base());
    }
}

