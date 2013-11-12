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
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <DGLNet/protocol/message.h>
#include <boost/asio/basic_streambuf_fwd.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>

namespace boost {
namespace asio {
typedef basic_streambuf<> streambuf;
namespace local {
class stream_protocol;
}
namespace ip {
class tcp;
}
}
}

namespace dglnet {

class TransportHeader;

template <class proto>
class TransportDetail;

class ITransport : public std::enable_shared_from_this<ITransport> {
   public:
    virtual ~ITransport() {}
    virtual void sendMessage(const Message* msg) = 0;
    virtual void poll() = 0;
    virtual void run_one() = 0;
    virtual void abort() = 0;

    std::shared_ptr<ITransport> get_shared_from_base() {
        return shared_from_this();
    }
};

template <class proto>
class Transport : public ITransport {
   public:
    Transport(MessageHandler* messageHandler);
    virtual ~Transport();
    virtual void sendMessage(const Message* msg) override;
    virtual void poll() override;
    virtual void run_one() override;
    virtual void abort() override;

   protected:
    void read();
    void notifyConnect();
    void notifyDisconnect(const boost::system::error_code& ec);
    virtual void notifyStartSend();
    virtual void notifyEndSend();

    std::shared_ptr<TransportDetail<proto> > m_detail;

   private:
    void writeQueue();

    void onReadHeader(TransportHeader* header,
                      const boost::system::error_code& ec);
    void onReadArchive(boost::asio::streambuf* stream,
                       const boost::system::error_code& ec);
    void onWrite(
        std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> >,
        const boost::system::error_code& ec);

    void onMessage(const Message& msg);

    std::shared_ptr<Transport<proto> > shared_from_this();

    MessageHandler* m_messageHandler;

    std::vector<std::pair<TransportHeader*, boost::asio::streambuf*> >
        m_WriteQueue;
    bool m_WriteReady;
    bool m_Abort;
};
}

#endif
