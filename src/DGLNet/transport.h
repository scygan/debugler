#include "message.h"
//#define BOOST_ASIO_DISABLE_IOCP
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace dglnet {

class  TransportHeader;

class Transport: public boost::enable_shared_from_this<Transport> {
public: 
    Transport(MessageHandler* messageHandler);
    virtual ~Transport() {}
    void sendMessage(const Message* msg);
    void poll();
    void run_one();
    void disconnect();
protected:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::socket m_socket;
    
    void read();
    void notifyDisconnect(const std::string& why = "");

    boost::shared_ptr<Transport> get_shared_from_base() {
        return shared_from_this();
    }

private:
    void onReadHeader(TransportHeader* header, const boost::system::error_code &ec);
    void onReadArchive(boost::asio::streambuf* stream, const boost::system::error_code &ec);
    void onWrite(TransportHeader* header, boost::asio::streambuf* stream, const boost::system::error_code &ec, std::size_t bytes_transferred);

    
    void onMessage(const Message& msg);

    MessageHandler* m_messageHandler;

};


}


