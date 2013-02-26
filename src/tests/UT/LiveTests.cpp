
#include "gtest/gtest.h"

#include <DGLGUI/dglprocess.h>

#include <DGLNet/client.h>

#include <windows.h>

namespace {

    // The fixture for testing class Foo.
    class LiveTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        LiveTest() {
            SetCurrentDirectory("..");
            DGLProcess* process = DGLProcess::Create(
                "C:\\Python27\\python.exe", "..", "..\\..\\src\\tests\\samples\\simple.py", 8888);
            while (!process->waitReady(100)) {}
            delete process;
        }

        virtual ~LiveTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }

        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:

        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }

        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }

        // Objects declared here can be used by all tests in the test case for Foo.
    };


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
        if (messageHandlerStub.mDisconnected) printf("Failing test, disconnection reason: %s\n", messageHandlerStub.mDisconnectedReason);
        ASSERT_FALSE(messageHandlerStub.mDisconnected);
        ASSERT_TRUE(msg != NULL);
        dglnet::HelloMessage * hello = dynamic_cast<dglnet::HelloMessage*>(msg);
        ASSERT_TRUE(hello != NULL);
        EXPECT_EQ(hello->m_ProcessName, "python.exe");
    }

    

}  // namespace


