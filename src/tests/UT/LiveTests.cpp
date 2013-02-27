
#include "gtest/gtest.h"

#include <DGLGUI/dglprocess.h>

#include <DGLNet/client.h>

#include <windows.h>
#include <boost/thread/thread.hpp> 

namespace {

    class LiveTest : public ::testing::Test {
    protected:

        LiveTest() {
            if (once) {
                SetCurrentDirectory("..");
                once = false;
            } 
            
            DGLProcess* process = DGLProcess::Create(
                "C:\\Python27\\python.exe", "..", "..\\..\\src\\tests\\samples\\simple.py", 8888);
            while (!process->waitReady(100)) {}
            delete process;
        }

        virtual ~LiveTest() {
        }

        virtual void SetUp() {}

        virtual void TearDown() {}

        static bool once;
    };
    bool LiveTest::once = true;


    namespace stubs {
        class Controller: public dglnet::IController {
        public:
            virtual void onSetStatus(std::string) {}
            virtual void onSocket() {}
        };

        class MessageHandler: public dglnet::MessageHandler {
            
            virtual void doHandle(const dglnet::HelloMessage& msg) { mLastMessage = new dglnet::HelloMessage(msg); }
            virtual void doHandle(const dglnet::ConfigurationMessage& msg) { mLastMessage = new dglnet::ConfigurationMessage(msg); }
            virtual void doHandle(const dglnet::BreakedCallMessage& msg) { mLastMessage = new dglnet::BreakedCallMessage(msg); }
            virtual void doHandle(const dglnet::ContinueBreakMessage& msg) { mLastMessage = new dglnet::ContinueBreakMessage(msg); }
            virtual void doHandle(const dglnet::QueryCallTraceMessage& msg) { mLastMessage = new dglnet::QueryCallTraceMessage(msg); }
            virtual void doHandle(const dglnet::CallTraceMessage& msg) { mLastMessage = new dglnet::CallTraceMessage(msg); }
            virtual void doHandle(const dglnet::QueryResourceMessage& msg) { mLastMessage = new dglnet::QueryResourceMessage(msg); }
            virtual void doHandle(const dglnet::ResourceMessage& msg) { mLastMessage = new dglnet::ResourceMessage(msg); }
            virtual void doHandle(const dglnet::SetBreakPointsMessage& msg) { mLastMessage = new dglnet::SetBreakPointsMessage(msg); }
            void doHandleDisconnect(const std::string &msg) { mDisconnected = true; mDisconnectedReason = msg; }

        public:
            MessageHandler():mLastMessage(NULL),mDisconnected(false) {};
            dglnet::Message* getLastMessage() {
                dglnet::Message* ret = mLastMessage;
                mLastMessage = NULL;
                return ret;
            }
            dglnet::Message* mLastMessage;
            bool mDisconnected; std::string mDisconnectedReason;
        };
    }


    TEST_F(LiveTest, connect_disconnect) {
        stubs::Controller controllerStub;
        stubs::MessageHandler messageHandlerStub;
        boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
        client->connectServer("127.0.0.1", "8888");
        dglnet::Message* msg;
        while (!(msg = messageHandlerStub.getLastMessage()) && !messageHandlerStub.mDisconnected) {
            client->run_one();
        }
        ASSERT_FALSE(messageHandlerStub.mDisconnected);
        ASSERT_TRUE(msg != NULL);
        dglnet::HelloMessage * hello = dynamic_cast<dglnet::HelloMessage*>(msg);
        ASSERT_TRUE(hello != NULL);
        EXPECT_EQ(hello->m_ProcessName, "python.exe");
        client->abort();
        EXPECT_TRUE(client.unique());
    }

   TEST_F(LiveTest, continue_break) {
       stubs::Controller controllerStub;
       stubs::MessageHandler messageHandlerStub;
       boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
       client->connectServer("127.0.0.1", "8888");
       dglnet::Message* msg;
       while (!(msg = messageHandlerStub.getLastMessage()) && !messageHandlerStub.mDisconnected) {
           client->run_one();
       }
       ASSERT_TRUE(msg != NULL);
       dglnet::HelloMessage * hello = dynamic_cast<dglnet::HelloMessage*>(msg);
       ASSERT_TRUE(hello != NULL);
       
       while (!dynamic_cast<dglnet::BreakedCallMessage*>(msg)) {
           while (!(msg = messageHandlerStub.getLastMessage()) && !messageHandlerStub.mDisconnected) {
               client->run_one();
           }
       }
       {
           dglnet::ContinueBreakMessage continueMsg(false);
           client->sendMessage(&continueMsg);
       }
       boost::this_thread::sleep(boost::posix_time::milliseconds(10));
       {
           dglnet::ContinueBreakMessage continueMsg(true);
           client->sendMessage(&continueMsg);
       }
       while (!dynamic_cast<dglnet::BreakedCallMessage*>(msg)) {
           while (!(msg = messageHandlerStub.getLastMessage()) && !messageHandlerStub.mDisconnected) {
               client->run_one();
           }
       }
       client->abort();
   }

    

}  // namespace


