#include<string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "client.h"


namespace dglnet {

    Client::Client(std::string host, std::string port, IController* controller):m_controller(controller) {
        boost::asio::ip::tcp::resolver resolver(m_io_service); 
        boost::asio::ip::tcp::resolver::query query(host, port);
        resolver.async_resolve(query, boost::bind(&Client::onResolve, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
        m_controller->onSetStatus("Looking up server...");
    }

    int Client::getSocketFD() {
        return m_socket.native_handle();
    }

    void Client::poll() {
        m_io_service.poll();
    }

    void Client::onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
        
        if (!err) {
            m_socket.async_connect(*endpoint_iterator, boost::bind(&Client::onConnect, this,
                boost::asio::placeholders::error));
            m_controller->onSetStatus("Connecting...");
        } else {
            m_controller->onInternalError(err.message());
        }
    }

    void Client::onConnect(const boost::system::error_code &err) {
        if (!err) {
            m_connected = true;
        } else {
            m_controller->onInternalError(err.message());
        }
    }

}

