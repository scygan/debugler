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

#include "client.h"
#include "transport_detail.h"

#include<string>
#include <boost/bind.hpp>

#include <boost/asio/placeholders.hpp>




namespace dglnet {

    Client::Client(MessageHandler* messageHandler):Transport(messageHandler) {}

    class ClientImpl: public Client {
    public: 

        ClientImpl(IController* controller, MessageHandler* messageHandler):Client(messageHandler),m_controller(controller), m_Resolver(Transport::m_detail->m_io_service) {}

        virtual void connectServer(std::string host, std::string port) {
            boost::asio::ip::tcp::resolver::query query(host, port);
            m_Resolver.async_resolve(query, boost::bind(&ClientImpl::onResolve, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator));
            m_controller->onSetStatus("Looking up server...");
        }

        virtual socket_fd_t getSocketFD() {
            return Transport::m_detail->m_socket.native_handle();
        }


    private:
        virtual void onResolve(const boost::system::error_code& err,
            boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
                if (!err) {
                    Transport::m_detail->m_socket.async_connect(*endpoint_iterator, boost::bind(&ClientImpl::onConnect, shared_from_this(),
                        boost::asio::placeholders::error));
                    m_controller->onSetStatus("Connecting...");
                    m_controller->onSocket();
                } else {
                    notifyDisconnect(err.message());
                }
        }

        virtual void onConnect(const boost::system::error_code &err) {
            if (!err) {
                m_controller->onSetStatus("Connected.");
                read();
            } else {
                notifyDisconnect(err.message());
            }
        }

        virtual void notifyStartSend() override {
            m_controller->onSocketStartSend();
        }

        virtual void notifyEndSend() override {
            m_controller->onSocketStopSend();
        }

        std::shared_ptr<ClientImpl> shared_from_this() {
            return std::static_pointer_cast<ClientImpl>(get_shared_from_base());
        }

        IController* m_controller;
        boost::asio::ip::tcp::resolver m_Resolver;
    };

    std::shared_ptr<Client> Client::Create(IController* controller, MessageHandler* messageHandler) {
        return std::make_shared<ClientImpl>(controller, messageHandler);
    }
}

