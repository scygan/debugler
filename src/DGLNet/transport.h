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


#include <DGLNet/protocol/message.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace dglnet {

class  TransportHeader;

class Transport: public boost::enable_shared_from_this<Transport> {
public: 
    Transport(MessageHandler* messageHandler);
    virtual ~Transport();
    void sendMessage(const Message* msg);
    void poll();
    void run_one();
    void abort();
protected:
    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::socket m_socket;
    
    void read();
    void notifyDisconnect(const std::string& why = "");
    virtual void notifyStartSend();
    virtual void notifyEndSend();

    boost::shared_ptr<Transport> get_shared_from_base() {
        return shared_from_this();
    }

private:
    void writeQueue();

    void onReadHeader(TransportHeader* header, const boost::system::error_code &ec);
    void onReadArchive(boost::asio::streambuf* stream, const boost::system::error_code &ec);
    void onWrite(std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> >, const boost::system::error_code &ec, std::size_t bytes_transferred);
    
    
    void onMessage(const Message& msg);

    MessageHandler* m_messageHandler;

    std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> > m_WriteQueue;
    bool m_WriteReady;
    bool m_Abort;

};


}


