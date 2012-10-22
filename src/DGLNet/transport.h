#include "message.h"
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace dglnet {

struct TransportHeader {
public:
    TransportHeader() {}
    TransportHeader(int size);
    int getSize();
private:
    int32_t m_size;
};

class Transport: public boost::enable_shared_from_this<Transport> {
public: 
    Transport();
    virtual ~Transport() {}
    void sendMessage(const Message* msg);
    void poll();

protected:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::socket m_socket;
    
    void read();

private:
    TransportHeader m_pendingHeader;    
    std::vector<char> m_pendingArchiveBuffer;

    void onReadHeader(const boost::system::error_code &ec);
    void onReadArchive(const boost::system::error_code &ec);
    void onWrite(const boost::system::error_code &ec, std::size_t bytes_transferred);

    

    
};


}


