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

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

namespace dglnet {

    //boost::asio::ip::tcp::endpoint
    template<class proto> struct type_dispatch { typedef int endpoint_t;  };
    template<> struct type_dispatch<boost::asio::ip::tcp> {  typedef boost::asio::ip::tcp::endpoint endpoint_t;  };
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    template<> struct type_dispatch<boost::asio::local::stream_protocol> {  typedef boost::asio::local::stream_protocol::endpoint endpoint_t;  };
#endif
   
    template<class proto>
    class ServerDetail {
        public:
        ServerDetail(const std::string& port, boost::asio::io_service& io_service);
       
        typename type_dispatch<proto>::endpoint_t m_endpoint;

        boost::asio::basic_socket_acceptor<proto> m_acceptor;

        unsigned short portFromStr(const std::string& str) {
            std::stringstream stream(str);
            unsigned short ret; 
            stream >> ret;
            return ret;
        }
    };

    template<>
    ServerDetail<boost::asio::ip::tcp>::ServerDetail(const std::string& port, boost::asio::io_service& io_service):m_endpoint(boost::asio::ip::tcp::v4(), portFromStr(port)), m_acceptor(io_service) {
        m_acceptor.open(m_endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(m_endpoint);
    }

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    template<>
    ServerDetail<boost::asio::local::stream_protocol>::ServerDetail(const std::string& port, boost::asio::io_service& io_service):m_endpoint(port), m_acceptor(io_service) {}
#endif

    template<class proto>
    Server<proto>::Server(const std::string& port, MessageHandler* handler):Transport<proto>(handler),m_detail(std::make_shared<ServerDetail<proto> >(port, Transport<proto>::m_detail->m_io_service)) {
        m_detail->m_acceptor.listen();
    }

    template<class proto>
    void Server<proto>::accept(bool wait) {
        if (wait) {
            boost::system::error_code ec;
            ec = m_detail->m_acceptor.accept(Transport<proto>::m_detail->m_socket, ec);
            if (!ec) {
                Transport<proto>::notifyConnect();
                m_detail->m_acceptor.close();
                Transport<proto>::read();
            } else {
                Transport<proto>::notifyDisconnect(ec);
            }
        } else {
            m_detail->m_acceptor.async_accept(Transport<proto>::m_detail->m_socket, std::bind(&Server::onAccept, shared_from_this(), std::placeholders::_1));
        }
    }

    template<class proto>
    void Server<proto>::onAccept(const boost::system::error_code &ec) {
        if (!ec) {
            Transport<proto>::notifyConnect();
            m_detail->m_acceptor.close();
            Transport<proto>::read();
        } else {
            Transport<proto>::notifyDisconnect(ec);
        }
    }

    template<class proto>
    std::shared_ptr<Server<proto> > Server<proto>::shared_from_this() {
        return std::static_pointer_cast<Server<proto> >(Transport<proto>::get_shared_from_base());
    }


    template class Server<boost::asio::ip::tcp>;
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    template class Server<boost::asio::local::stream_protocol>;
#endif
}

