#include<string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "client.h"


namespace dglnet {
    Client::Client(std::string host, std::string port) {
        boost::asio::ip::tcp::resolver resolver(io_service); 
        boost::asio::ip::tcp::resolver::query query(host, port);
        resolver.async_resolve(query, boost::bind());
    }
}

