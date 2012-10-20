#include "message.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace dglnet {

    class Transport: public boost::enable_shared_from_this<Transport> {
public: 
    Transport();
    void sendMessage(const Message* msg);
    void poll();

protected:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::socket m_socket;
    bool m_connected;
    
};


}


