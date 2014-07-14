/* Copyright (C) 2014 Slawomir Cygan <slawomir.cygan@gmail.com>
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

#include "transport.h"

#include <boost/asio/io_service.hpp>

#pragma warning(push)
//user defined binary operator ',' exists but no overload could convert all operands, default built-in binary operator ',' used
#pragma warning(disable : 4913)
#include <boost/asio/basic_stream_socket.hpp>
#pragma warning(pop)


namespace dglnet {

template <class proto>
class TransportDetail {
   public:
    TransportDetail();
    boost::asio::io_service m_io_service;
    boost::asio::basic_stream_socket<proto> m_socket;
};
}
