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

#ifndef _FWD_H
#define _FWD_H

class RetValue;
class CalledEntryPoint;

namespace dglnet {

class Message;
class MessageHandler;

class DGLRequest;

namespace message {
    // generic messages
    class Hello;
    class Configuration;
    class BreakedCall;
    class ContinueBreak;
    class Terminate;
    class QueryCallTrace;
    class CallTrace;

    class Request;
    class RequestReply;

    class SetBreakPoints;

    namespace utils {
        class ContextReport;
        class ReplyBase;
    }
}

class DGLResource;

namespace resource {

    class DGLPixelRectangle;

    namespace utils {
        class StateItem;
    }
}

}

#endif
