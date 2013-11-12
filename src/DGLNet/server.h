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

#ifndef _SERVER_H
#define _SERVER_H

#include "DGLNet/transport.h"
#include <mutex>

namespace dglnet {

template <class proto>
class ServerDetail;

template <class proto>
class Server : public Transport<proto> {
   public:
    Server(const std::string& port, MessageHandler*);

    virtual void accept(bool wait);

   private:
    virtual void onAccept(const boost::system::error_code& ec);

    std::shared_ptr<Server<proto> > shared_from_this();

    std::shared_ptr<ServerDetail<proto> > m_detail;
};

typedef Server<boost::asio::ip::tcp> ServerTcp;
typedef Server<boost::asio::local::stream_protocol> ServerUnixDomain;
}

#endif
