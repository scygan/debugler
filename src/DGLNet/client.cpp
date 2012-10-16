#include<string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "client.h"


namespace dglnet {
    Client::Client(std::string host, std::string port) {
        boost::asio::ip::tcp::resolver resolver(m_io_service); 
        boost::asio::ip::tcp::resolver::query query(host, port);
        resolver.async_resolve(query, boost::bind(&Client::onResolve, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
        setStatus("Looking up server...");
    }

    void Client::onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
        
        if (!err) {
            m_socket.async_connect(*endpoint_iterator, boost::bind(&Client::onConnect, this,
                boost::asio::placeholders::error));
            setStatus("Connecting...");
        }

        setStatus("Failed");
    }

    void Client::onConnect(const boost::system::error_code &ec) {
        m_connected = true;
    }

}

