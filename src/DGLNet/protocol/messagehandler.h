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
#ifndef _MESSAGE_HANDLER_H
#define _MESSAGE_HANDLER_H

#include <DGLNet/protocol/fwd.h>

namespace dglnet {

class MessageHandler {
   public:
    virtual void doHandleHello(const message::Hello&);
    virtual void doHandleConfiguration(const message::Configuration&);
    virtual void doHandleBreakedCall(const message::BreakedCall&);
    virtual void doHandleContinueBreak(const message::ContinueBreak&);
    virtual void doHandleTerminate(const message::Terminate&);
    virtual void doHandleQueryCallTrace(const message::QueryCallTrace&);
    virtual void doHandleCallTrace(const message::CallTrace&);
    virtual void doHandleRequest(const message::Request&);
    virtual void doHandleRequestReply(const message::RequestReply&);
    virtual void doHandleSetBreakPoints(const message::SetBreakPoints&);

    virtual void doHandleConnect() = 0;
    virtual void doHandleDisconnect(const std::string& why) = 0;
    virtual ~MessageHandler() {}

   private:
    void unsupported();
};

}
#endif
