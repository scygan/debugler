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

#include <boost/bind.hpp>

namespace dglnet {

    Server::Server(unsigned short port, MessageHandler* handler):Transport(handler),m_endpoint(boost::asio::ip::tcp::v4(), port), m_acceptor(m_io_service) {
        m_acceptor.open(m_endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(m_endpoint);
        m_acceptor.listen();
    }
    void Server::accept() {
        boost::system::error_code ec;
        ec = m_acceptor.accept(m_socket, ec);
        if (ec) {
            notifyDisconnect(ec.message());
        }
        m_acceptor.close();
        read();
    }

    void Server::lock() {
        m_mutex.lock();
    }    

    void Server::unlock() {
        m_mutex.unlock();
    }  
}

