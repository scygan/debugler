/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <boost/serialization/export.hpp> 
#define REGISTER_CLASS(X) BOOST_CLASS_EXPORT(X)
#include "protocol/message.h"
#include "protocol/resource.h"
#include "protocol/request.h"
#undef REGISTER_CLASS


#include "transport.h"
#include "transport_detail.h"

#include <portable_archive/portable_oarchive.hpp>
#pragma warning(push)
#pragma warning(disable:4244) //'argument' : conversion from 'std::streamsize' to 'size_t', possible loss of data basic_binary_iprimitive.hpp	181
#include <portable_archive/portable_iarchive.hpp>
#pragma warning(pop)
#include <boost/serialization/set.hpp>
#pragma warning(push)
#pragma warning(disable:4512) //'boost::serialization::variant_save_visitor<Archive>' : assignment operator could not be generated

#include <boost/serialization/variant.hpp>
#pragma warning(pop)
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/streambuf.hpp>

#include <sstream>

namespace dglnet {

    class TransportHeader {
    public:
        TransportHeader() {}
        TransportHeader(value_t size):m_size(size) {}
        int getSize() {
            return m_size;
        };
    private:
        value_t m_size;
    };

    TransportDetail::TransportDetail():m_socket(m_io_service) {};

    Transport::Transport(MessageHandler* handler):m_detail(std::make_shared<TransportDetail>()),m_messageHandler(handler),m_WriteReady(true),m_Abort(false) {}

    Transport::~Transport() {
        if (!m_Abort) abort();
    }

    void Transport::abort() {
        m_Abort = true;
        try {
            m_detail->m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            m_detail->m_socket.close();
            while (m_detail->m_io_service.run_one()) {}
        } catch( ... ) {}
    }
    
    void Transport::poll() {
        while (m_detail->m_io_service.poll());
    }

    void Transport::run_one() {
        m_detail->m_io_service.run_one();
    }

    void Transport::read() {
        TransportHeader* header = new TransportHeader();
        boost::asio::async_read(m_detail->m_socket, boost::asio::buffer(header, sizeof(TransportHeader)), std::bind(&Transport::onReadHeader, shared_from_this(), header,
            std::placeholders::_1));
    }

    void Transport::onReadHeader(TransportHeader* header, const boost::system::error_code &ec) {
        if (ec) {
            notifyDisconnect(ec.message());
        } else {
            boost::asio::streambuf*  stream = new boost::asio::streambuf;
            stream->prepare(header->getSize());
            boost::asio::async_read(m_detail->m_socket, *stream, boost::asio::transfer_exactly(header->getSize()), std::bind(&Transport::onReadArchive, shared_from_this(),
                stream, std::placeholders::_1));
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
                eos::portable_iarchive archive(iArchiveStream);
                archive >> msg;
            }

            onMessage(*msg);

            delete msg;

            read();
        }
        delete stream;
    }

    void Transport::sendMessage(const Message* msg) {
        //create and push new stream to queue
        boost::asio::streambuf* stream = new boost::asio::streambuf;        
        {
            std::ostream oArchiveStream(stream);
            eos::portable_oarchive archive(oArchiveStream);
            archive << msg; 
        }

        TransportHeader* header = new TransportHeader(static_cast<value_t>(stream->size()));

        m_WriteQueue.push_back(std::pair<TransportHeader*, boost::asio::streambuf*>(header, stream));

        if (m_WriteReady) {
            notifyStartSend();
            m_WriteReady = false;
            writeQueue();
        }
    }
    
    void Transport::writeQueue() {
        m_WriteReady = false;
        
        std::vector<boost::asio::const_buffer> buffers(m_WriteQueue.size() * 2);

        for (size_t i =0; i < m_WriteQueue.size(); i++) {
            buffers[2 * i] = boost::asio::const_buffer(m_WriteQueue[i].first, sizeof(TransportHeader));
            buffers[2 * i + 1] = m_WriteQueue[i].second->data();
        }

        std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> > sentData;
        std::swap(m_WriteQueue, sentData);
        boost::asio::async_write(m_detail->m_socket, buffers, std::bind(&Transport::onWrite, shared_from_this(),
                sentData, std::placeholders::_1));
    }

    void Transport::onWrite(std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> > sentData, const boost::system::error_code &ec) {
        for (size_t i = 0; i < sentData.size(); i++) {
            delete sentData[i].first;
            delete sentData[i].second;
        }

        if (m_WriteQueue.size()) {
            writeQueue();
        } else {
            m_WriteReady = true;
            notifyEndSend();
        }
        
        if (ec) {
            notifyDisconnect(ec.message());
        }
    }

    void Transport::onMessage(const Message& msg) {
        msg.handle(m_messageHandler);
    }

    void Transport::notifyDisconnect(const std::string& why) {
        if (m_Abort) return;
        m_messageHandler->doHandleDisconnect(why);
    }
    
    void Transport::notifyStartSend() {}

    void Transport::notifyEndSend() {}
}
