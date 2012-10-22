#include "transport.h"
#include "message.h"


#include <sstream>
#include <boost/bind.hpp>


//TODO: this one is horrible. very.
BOOST_CLASS_EXPORT(dglnet::Message)
BOOST_CLASS_EXPORT_GUID(dglnet::CurrentCallStateMessage, "dglnet::CurrentCallStateMessage");


namespace dglnet {

    TransportHeader::TransportHeader(int size):m_size(size) {}

    int TransportHeader::getSize() { return m_size; }

    Transport::Transport():m_socket(m_io_service) {}
    
    void Transport::poll() {
        while (m_io_service.poll());
    }

    void Transport::read() {
        boost::asio::async_read(m_socket, boost::asio::buffer(&m_pendingHeader, sizeof(m_pendingHeader)), boost::bind(&Transport::onReadHeader, this,
            boost::asio::placeholders::error));
    }

    void Transport::onReadHeader(const boost::system::error_code &ec) {
        assert(!ec);
        m_pendingArchiveBuffer.resize(m_pendingHeader.getSize());
        boost::asio::async_read(m_socket, boost::asio::buffer(m_pendingArchiveBuffer), boost::bind(&Transport::onReadArchive, this,
            boost::asio::placeholders::error));
    }

    void Transport::onReadArchive(const boost::system::error_code &ec) {
        assert(!ec);
        std::string iArchiveString(&m_pendingArchiveBuffer[0], m_pendingHeader.getSize());
        std::istringstream iArchiveStream(iArchiveString, std::ios_base::binary);
        assert(iArchiveStream.good());
        Message* msg;
        boost::archive::binary_iarchive archive(iArchiveStream);
        archive >> msg;
        read();
    }

    void Transport::sendMessage(const Message* msg) {
        boost::asio::streambuf buffArchive;
        
        {
            std::ostream oArchiveStream(&buffArchive);
            boost::archive::binary_oarchive archive(oArchiveStream);
            archive << msg; 
        }

        TransportHeader header[1] = { TransportHeader(buffArchive.size()) };

        std::vector<boost::asio::const_buffer> buffersToSend;
        buffersToSend.push_back(boost::asio::buffer(header));
        buffersToSend.push_back(buffArchive.data());

        boost::asio::async_write(m_socket, buffersToSend, boost::bind(&Transport::onWrite, this,
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void Transport::onWrite(const boost::system::error_code &ec, std::size_t bytes_transferred) {
        assert(!ec);
    }

}