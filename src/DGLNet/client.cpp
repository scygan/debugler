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


#include<string>
#include <boost/bind.hpp>

#include <boost/asio/placeholders.hpp>

#include "client.h"


namespace dglnet {

    Client::Client(IController* controller, MessageHandler* handler):Transport(handler),m_controller(controller), m_Resolver(m_io_service) {}

    void Client::connectServer(std::string host, std::string port) {
        boost::asio::ip::tcp::resolver::query query(host, port);
        m_Resolver.async_resolve(query, boost::bind(&Client::onResolve, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::iterator));
        m_controller->onSetStatus("Looking up server...");
    }

    Client::socket_fd_t Client::getSocketFD() {
        return m_socket.native_handle();
    }

    void Client::onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
        if (!err) {
            m_socket.async_connect(*endpoint_iterator, boost::bind(&Client::onConnect, shared_from_this(),
                boost::asio::placeholders::error));
            m_controller->onSetStatus("Connecting...");
            m_controller->onSocket();
        } else {
            notifyDisconnect(err.message());
        }
    }

    void Client::onConnect(const boost::system::error_code &err) {
        if (!err) {
            m_controller->onSetStatus("Connected.");
            read();
        } else {
            notifyDisconnect(err.message());
        }
    }

    void Client::notifyStartSend() {
        m_controller->onSocketStartSend();
    }

    void Client::notifyEndSend() {
        m_controller->onSocketStopSend();
    }


    boost::shared_ptr<Client> Client::shared_from_this() {
        return boost::static_pointer_cast<Client>(get_shared_from_base());
    }
}

