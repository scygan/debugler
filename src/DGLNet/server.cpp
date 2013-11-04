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
    void Server::accept(bool wait) {
        if (wait) {
            boost::system::error_code ec;
            ec = m_detail->m_acceptor.accept(Transport::m_detail->m_socket, ec);
            if (!ec) {
                notifyConnect();
                m_detail->m_acceptor.close();
                read();
            } else {
                notifyDisconnect(ec);
            }
        } else {
            m_detail->m_acceptor.async_accept(Transport::m_detail->m_socket, std::bind(&Server::onAccept, shared_from_this(), std::placeholders::_1));
        }
    }

    void Server::onAccept(const boost::system::error_code &ec) {
        if (!ec) {
            notifyConnect();
            m_detail->m_acceptor.close();
            read();
        } else {
            notifyDisconnect(ec);
        }
    }

    std::mutex& Server::getMtx() {
        return m_mutex;
    }

    std::shared_ptr<Server> Server::shared_from_this() {
        return std::static_pointer_cast<Server>(get_shared_from_base());
    }
}

