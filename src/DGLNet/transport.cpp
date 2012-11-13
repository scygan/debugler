
#include "transport.h"

//for boost serialization
#pragma warning(disable:4244 4308)

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp> 
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>


#include <sstream>
#include <boost/bind.hpp>


//TODO: this one is horrible. very.
BOOST_CLASS_EXPORT(dglnet::Message)
BOOST_CLASS_EXPORT_GUID(dglnet::BreakedCallMessage, "dglnet::CurrentCallStateMessage");
BOOST_CLASS_EXPORT_GUID(dglnet::ContinueBreakMessage, "dglnet::ContinueBreakMessage");
BOOST_CLASS_EXPORT_GUID(dglnet::QueryCallTraceMessage, "dglnet::QueryCallTraceMessage");
BOOST_CLASS_EXPORT_GUID(dglnet::CallTraceMessage, "dglnet::CallTraceMessage");
BOOST_CLASS_EXPORT_GUID(dglnet::QueryTextureMessage, "dglnet::QueryTextureMessage");
BOOST_CLASS_EXPORT_GUID(dglnet::TextureMessage, "dglnet::TextureMessage");


namespace dglnet {

    class TransportHeader {
    public:
        TransportHeader() {}
        TransportHeader(int size):m_size(size) {}
        int getSize() {
            return m_size;
        };
    private:
        int32_t m_size;
    };

    Transport::Transport(MessageHandler* handler):m_socket(m_io_service),m_messageHandler(handler) {}

    void Transport::disconnect() {
        try {
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        } catch( ... ) {}
        m_socket.close();
        m_io_service.stop();
        m_io_service.run();
    }
    
    void Transport::poll() {
        while (m_io_service.poll());
    }

    void Transport::run_one() {
        m_io_service.run_one();
    }

    void Transport::read() {
        TransportHeader* header = new TransportHeader();
        boost::asio::async_read(m_socket, boost::asio::buffer(header, sizeof(TransportHeader)), boost::bind(&Transport::onReadHeader, shared_from_this(), header,
            boost::asio::placeholders::error));
    }

    void Transport::onReadHeader(TransportHeader* header, const boost::system::error_code &ec) {
        if (ec) {
            notifyDisconnect(ec.message());
        } else {
            boost::asio::streambuf*  stream = new boost::asio::streambuf;
            stream->prepare(header->getSize());
            boost::asio::async_read(m_socket, *stream, boost::asio::transfer_exactly(header->getSize()), boost::bind(&Transport::onReadArchive, shared_from_this(),
                stream, boost::asio::placeholders::error));
        }
        delete header;
    }

    void Transport::onReadArchive(boost::asio::streambuf* stream, const boost::system::error_code &ec) {
        if (ec) {
            notifyDisconnect(ec.message());
        } else {
            std::istream iArchiveStream(stream);
            assert(iArchiveStream.good());

            Message* msg;
            {
                boost::archive::binary_iarchive archive(iArchiveStream);
                archive >> msg;
            }

            onMessage(*msg);

            read();
        }
        delete stream;
    }

    void Transport::sendMessage(const Message* msg) {
        boost::asio::streambuf* stream = new boost::asio::streambuf();
        
        {
            std::ostream oArchiveStream(stream);
            boost::archive::binary_oarchive archive(oArchiveStream);
            archive << msg; 
        }

        TransportHeader* header = new TransportHeader(stream->size());

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::const_buffer(header, sizeof(TransportHeader)));
        buffers.push_back(stream->data());


        boost::asio::async_write(m_socket, buffers, boost::bind(&Transport::onWrite, shared_from_this(),
                header, stream, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void Transport::onWrite(TransportHeader* header, boost::asio::streambuf* stream, const boost::system::error_code &ec, std::size_t bytes_transferred) {
        delete header;
        delete stream;
        if (ec) {
            notifyDisconnect(ec.message());
        }
    }

    void Transport::onMessage(const Message& msg) {
        msg.handle(m_messageHandler);
    }

    void Transport::notifyDisconnect(const std::string& why) {
        try {
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        } catch (...) {}
        m_socket.close();
        m_io_service.stop();
        m_messageHandler->doHandleDisconnect(why);
    }


}