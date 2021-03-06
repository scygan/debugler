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

#include <string>

#include "transport.h"

namespace dglnet {

class IController {
   public:
    virtual void onSetStatus(std::string) = 0;
    virtual void onSocket() = 0;
    virtual void onSocketStartSend() = 0;
    virtual void onSocketStopSend() = 0;

    virtual ~IController() {}
};

class Client : public Transport<boost::asio::ip::tcp> {
   public:
    virtual ~Client() {}
    virtual void connectServer(std::string host, std::string port) = 0;

    typedef uint64_t socket_fd_t;
    virtual socket_fd_t getSocketFD() = 0;
    static std::shared_ptr<Client> Create(IController* controller,
                                          MessageHandler* messageHandler);

   protected:
    Client(MessageHandler* messageHandler);
};

}    // namespace dglnet
