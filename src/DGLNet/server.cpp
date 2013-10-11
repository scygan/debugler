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


#include "server.h"

#include "transport_detail.h"
#include <boost/bind.hpp>

namespace dglnet {

    class ServerDetail {
        public:
        ServerDetail(short port, boost::asio::io_service& io_service):m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(io_service) {}
        boost::asio::ip::tcp::endpoint m_endpoint;
        boost::asio::ip::tcp::acceptor m_acceptor;
    };

    Server::Server(unsigned short port, MessageHandler* handler):Transport(handler),m_detail(std::make_shared<ServerDetail>(port, Transport::m_detail->m_io_service)) {
        m_detail->m_acceptor.open(m_detail->m_endpoint.protocol());
        m_detail->m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_detail->m_acceptor.bind(m_detail->m_endpoint);
        m_detail->m_acceptor.listen();
    }
    void Server::accept() {
        boost::system::error_code ec;
        ec = m_detail->m_acceptor.accept(Transport::m_detail->m_socket, ec);
        if (ec) {
            notifyDisconnect(ec.message());
        }
        m_detail->m_acceptor.close();
        read();
    }

    std::mutex& Server::getMtx() {
        return m_mutex;
    }
}

