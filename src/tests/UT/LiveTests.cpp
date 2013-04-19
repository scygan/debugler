
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
                "C:\\Python27\\python.exe", "..", "..\\..\\src\\tests\\samples\\simple.py", 8888, false);
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

    namespace utils {
        template<typename T>
        T* receiveMessage(dglnet::Transport* transport, stubs::MessageHandler& handler) {
            dglnet::Message* msg = NULL; 
            do {
                transport->run_one();
            } while (!handler.mDisconnected && !(msg = handler.getLastMessage()));
            EXPECT_EQ(false, handler.mDisconnected);
            if (!msg || !dynamic_cast<T*>(msg)) {
                return NULL;
            }
            return dynamic_cast<T*>(msg);
        }

        template<typename T>
        T* receiveUntilMessage(dglnet::Transport* transport, stubs::MessageHandler& handler) {
            dglnet::Message* msg = NULL; 
            do {
                transport->run_one();
                if (!handler.mDisconnected) {
                    msg = handler.getLastMessage();
                }
            } while (!handler.mDisconnected && (!msg || !(dynamic_cast<T*>(msg))));
            EXPECT_EQ(false, handler.mDisconnected);
            if (!msg || !dynamic_cast<T*>(msg)) {
                return NULL;
            }
            return dynamic_cast<T*>(msg);
        }

    }


    TEST_F(LiveTest, connect_disconnect) {
        stubs::Controller controllerStub;
        stubs::MessageHandler messageHandlerStub;
        boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
        client->connectServer("127.0.0.1", "8888");
        dglnet::HelloMessage * hello = utils::receiveMessage<dglnet::HelloMessage>(client.get(), messageHandlerStub);
        ASSERT_TRUE(hello != NULL);
        EXPECT_EQ("python.exe", hello->m_ProcessName);
        client->abort();
        EXPECT_TRUE(client.unique());
    }

   TEST_F(LiveTest, continue_break) {
       stubs::Controller controllerStub;
       stubs::MessageHandler messageHandlerStub;
       boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
       client->connectServer("127.0.0.1", "8888");

       utils::receiveMessage<dglnet::HelloMessage>(client.get(), messageHandlerStub);
       dglnet::BreakedCallMessage* breaked = utils::receiveMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
       ASSERT_TRUE(breaked != NULL);

       EXPECT_EQ(wglGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
       EXPECT_EQ(0, breaked->m_TraceSize);
       EXPECT_EQ(0, breaked->m_CtxReports.size());
       EXPECT_EQ(0, breaked->m_CurrentCtx);

       do {
           {
               dglnet::ContinueBreakMessage continueMsg(false);
               client->sendMessage(&continueMsg);
           }
           boost::this_thread::sleep(boost::posix_time::milliseconds(10));
           {
               dglnet::ContinueBreakMessage continueMsg(true);
               client->sendMessage(&continueMsg);
           }

           breaked = utils::receiveMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
           ASSERT_TRUE(breaked != NULL);
       } while (!breaked->m_CurrentCtx);       
       EXPECT_NE(0, breaked->m_TraceSize);
       EXPECT_EQ(1, breaked->m_CtxReports.size());
       EXPECT_NE(0, breaked->m_CurrentCtx);
       client->abort();
   }

   TEST_F(LiveTest, breakpoint) {
       stubs::Controller controllerStub;
       stubs::MessageHandler messageHandlerStub;
       boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
       client->connectServer("127.0.0.1", "8888");

       utils::receiveMessage<dglnet::HelloMessage>(client.get(), messageHandlerStub);
       dglnet::BreakedCallMessage* breaked = utils::receiveMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
       ASSERT_TRUE(breaked != NULL);

       EXPECT_EQ(wglGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
       EXPECT_EQ(0, breaked->m_TraceSize);
       EXPECT_EQ(0, breaked->m_CtxReports.size());
       EXPECT_EQ(0, breaked->m_CurrentCtx);

       {
           std::set<Entrypoint> breakpoints;
           breakpoints.insert(glDrawArrays_Call);
           dglnet::SetBreakPointsMessage breakPointMessage(breakpoints);
           client->sendMessage(&breakPointMessage);
       }
       {
           dglnet::ContinueBreakMessage continueMsg(false);
           client->sendMessage(&continueMsg);
       }

       breaked = utils::receiveMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
       ASSERT_TRUE(breaked != NULL);
        
       EXPECT_EQ(glDrawArrays_Call, breaked->m_entryp.getEntrypoint());

       client->abort();
   }


   TEST_F(LiveTest, entryp_retvals) {
       stubs::Controller controllerStub;
       stubs::MessageHandler messageHandlerStub;
       boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&controllerStub, &messageHandlerStub));
       client->connectServer("127.0.0.1", "8888");

       dglnet::BreakedCallMessage* breaked = utils::receiveUntilMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
       ASSERT_TRUE(breaked != NULL);
              
       {
           //set breakpoints && disable other breaking stuff
           std::set<Entrypoint> breakpoints;
           breakpoints.insert(wglCreateContext_Call);
           breakpoints.insert(glGetError_Call);
           breakpoints.insert(glCreateShader_Call);
           dglnet::SetBreakPointsMessage breakPointMessage(breakpoints);
           client->sendMessage(&breakPointMessage);

           dglnet::ConfigurationMessage config(DGLConfiguration(false, false, false));
           client->sendMessage(&config);
       }
       {
           //continue
           dglnet::ContinueBreakMessage continueMsg(false);
           client->sendMessage(&continueMsg);
       }

       bool wglCreateContext_done = false;
       bool glGetError_done = false;
       bool glCreateShader_done = false;

       while (!wglCreateContext_done || !glGetError_done || !glCreateShader_done) {
           breaked = utils::receiveUntilMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub);
           ASSERT_TRUE(breaked != NULL);

           {
               //we should be breaked on one of earlier breakpoints
               ASSERT_TRUE(
                   breaked->m_entryp.getEntrypoint() == wglCreateContext_Call ||
                   breaked->m_entryp.getEntrypoint() == glGetError_Call ||
                   breaked->m_entryp.getEntrypoint() == glCreateShader_Call
                   );
               
               {
                   //advance one call
                   dglnet::ContinueBreakMessage step(dglnet::ContinueBreakMessage::STEP_CALL);
                   client->sendMessage(&step);
                   ASSERT_TRUE(utils::receiveUntilMessage<dglnet::BreakedCallMessage>(client.get(), messageHandlerStub) != NULL);
               }
               
               {
                   //query callTrace with one entryp
                   dglnet::QueryCallTraceMessage queryCallTrace(0, 1);
                   client->sendMessage(&queryCallTrace);
               }

               dglnet::CallTraceMessage* callTrace = utils::receiveUntilMessage<dglnet::CallTraceMessage>(client.get(), messageHandlerStub);
               ASSERT_TRUE(callTrace != NULL);

               HGLRC ctx;
               GLenum error;
               GLuint shader;

               switch (callTrace->m_Trace[0].getEntrypoint()) {
                   case wglCreateContext_Call:
                       wglCreateContext_done = true;
                       callTrace->m_Trace[0].getRetVal().get(ctx);
                       EXPECT_TRUE(ctx != NULL);
                       break;
                   case glGetError_Call:
                       glGetError_done = true;
                       callTrace->m_Trace[0].getRetVal().get(error);
                       EXPECT_EQ(GL_NO_ERROR, error);
                       break;
                   case glCreateShader_Call:
                       glCreateShader_done = true;
                       callTrace->m_Trace[0].getRetVal().get(shader);
                       EXPECT_TRUE(shader != 0);
                       break;
                   default:
                       ASSERT_TRUE(!"breaked at wrong entryp");
               }

               //continue execution
               dglnet::ContinueBreakMessage continueMsg(false);
               client->sendMessage(&continueMsg);
           }
       }

       client->abort();
   }

    

}  // namespace


