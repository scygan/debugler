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


#include "gtest/gtest.h"
#include <DGLNet/client.h>
#include <DGLGui/dglprocess.h>
#include <DGLCommon/gl-headers.h>

#include <DGLNet/protocol/request.h>
#include <DGLNet/protocol/resource.h>

#include <thread>
#include <chrono>

namespace {

    class LiveTest : public ::testing::Test {
    protected:

        LiveTest() {
            if (once) {
#ifdef _WIN32
                SetCurrentDirectory("..");
#endif
                once = false;
            }


        }

        virtual ~LiveTest() {}

        class Controller: public dglnet::IController {
        public:
            virtual void onSetStatus(std::string) {}
            virtual void onSocket() {}
            virtual void onSocketStartSend() {}
            virtual void onSocketStopSend() {}
        } m_Controller;

        public:
        class MessageHandler: public dglnet::MessageHandler {
#define MAKE_HANDLER(T) \
            virtual void doHandle##T(const dglnet::message::T& msg) override { mLastMessage = new dglnet::message::T(msg); }

            MAKE_HANDLER(Hello);
            MAKE_HANDLER(Configuration);
            MAKE_HANDLER(BreakedCall);
            MAKE_HANDLER(ContinueBreak);
            MAKE_HANDLER(QueryCallTrace);
            MAKE_HANDLER(CallTrace);
            MAKE_HANDLER(Request);
            MAKE_HANDLER(RequestReply);
            MAKE_HANDLER(SetBreakPoints);
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
        
        private:
        MessageHandler m_MessageHandler;

        public:
        MessageHandler& getMessageHandler() { return m_MessageHandler; }

        boost::shared_ptr<dglnet::Client> getClientFor(std::string sampleName) {
            DGLDebugeeProcess* process = DGLDebugeeProcess::Create(
#ifdef _WIN32
                "C:\\Python27\\python.exe", "..", "..\\..\\src\\tests\\samples\\" + sampleName,
#else
                "python", "..", CMAKE_CURRENT_SOURCE_DIR"/../samples/" + sampleName,
#endif
                8888, false);
            while (!process->waitReady(100)) {}
            delete process;

            boost::shared_ptr<dglnet::Client> client(new dglnet::Client(&m_Controller, &m_MessageHandler));
            client->connectServer("127.0.0.1", "8888");

            return client;
        }

        DGLConfiguration getUsualConfig() {
            DGLConfiguration config;
            config.m_BreakOnGLError = false;
            config.m_BreakOnDebugOutput = false;
            config.m_BreakOnCompilerError = false;
            return config;
        }

        static bool once;
    };
    bool LiveTest::once = true;

    namespace utils {
        template<typename T>
        T* receiveMessage(dglnet::Transport* transport, LiveTest::MessageHandler& handler) {
            dglnet::Message* msg = NULL; 
            do {
                transport->run_one();
            } while (!handler.mDisconnected && !((msg = handler.getLastMessage()) != NULL));
            EXPECT_EQ(false, handler.mDisconnected);
            if (!msg || !dynamic_cast<T*>(msg)) {
                return NULL;
            }
            return dynamic_cast<T*>(msg);
        }

        template<typename T>
        T* receiveUntilMessage(dglnet::Transport* transport, LiveTest::MessageHandler& handler) {
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

        dglnet::message::BreakedCall* runUntilEntryPoint(boost::shared_ptr<dglnet::Client> client, LiveTest::MessageHandler& handler, Entrypoint entryp) {
            //set break point
            {
                std::set<Entrypoint> breakpoints;
                breakpoints.insert(entryp);
                dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
                client->sendMessage(&breakPointMessage);
            }
            
            //continue
            {
                dglnet::message::ContinueBreak continueMsg(false);
                client->sendMessage(&continueMsg);
            }

            //wait for break
            dglnet::message::BreakedCall* ret = utils::receiveMessage<dglnet::message::BreakedCall>(client.get(), handler);

            //clear breakpoints
            {
                std::set<Entrypoint> breakpoints;
                dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
                client->sendMessage(&breakPointMessage);
            }

            EXPECT_TRUE(ret != NULL);
            EXPECT_EQ(entryp, ret->m_entryp.getEntrypoint());

            return ret;
        }


        void checkColor(GLubyte* ptr, int width, int height, int rowBytes, int r, int g, int b, int a) {
            for (int y = 0; y < height; y++) {
                GLubyte* rowPtr = y * rowBytes + ptr;
                for (int x = 0; x < width; x++) {
                    if (x > 0.3 * width && x < 0.7 * width && y > 0.3 * height && y < 0.7 * height) {
                        ASSERT_TRUE(abs(r - rowPtr[4 * x + 0]) <= 1);
                        ASSERT_TRUE(abs(g - rowPtr[4 * x + 1]) <= 1);
                        ASSERT_TRUE(abs(b - rowPtr[4 * x + 2]) <= 1);
                        ASSERT_TRUE(abs(a - rowPtr[4 * x + 3]) <= 1);
                    }
                    if (x < 0.2 * width || x > 0.8 * width || y < 0.2 * height || y > 0.8 * height) {
                        ASSERT_TRUE(abs(rowPtr[4 * x + 0]) <= 1);
                        ASSERT_TRUE(abs(rowPtr[4 * x + 1]) <= 1);
                        ASSERT_TRUE(abs(rowPtr[4 * x + 2]) <= 1);
                        ASSERT_TRUE(abs(rowPtr[4 * x + 3]) <= 1);
                    }
                }
            }
        }

    }


    TEST_F(LiveTest, connect_disconnect) {

        boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

        dglnet::message::Hello * hello = utils::receiveMessage<dglnet::message::Hello>(client.get(), getMessageHandler());
        ASSERT_TRUE(hello != NULL);
#ifdef _WIN32
        EXPECT_EQ("python.exe", hello->m_ProcessName);
#else
        EXPECT_TRUE(std::string::npos != hello->m_ProcessName.find("python"));
#endif
        client->abort();
        EXPECT_TRUE(client.unique());
    }

   TEST_F(LiveTest, continue_break) {
       boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

       utils::receiveMessage<dglnet::message::Hello>(client.get(), getMessageHandler());
       dglnet::message::BreakedCall* breaked = utils::receiveMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
       EXPECT_EQ(wglGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
#else
       EXPECT_EQ(glXGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
#endif
       EXPECT_EQ(0, breaked->m_TraceSize);
       EXPECT_EQ(0, breaked->m_CtxReports.size());
       EXPECT_EQ(0, breaked->m_CurrentCtx);

       do {
           {
               dglnet::message::ContinueBreak continueMsg(false);
               client->sendMessage(&continueMsg);
           }
           std::this_thread::sleep_for(std::chrono::milliseconds(10));
           {
               dglnet::message::ContinueBreak continueMsg(true);
               client->sendMessage(&continueMsg);
           }

           breaked = utils::receiveMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
           ASSERT_TRUE(breaked != NULL);
       } while (!breaked->m_CurrentCtx);       
       EXPECT_NE(0, breaked->m_TraceSize);
       EXPECT_EQ(1, breaked->m_CtxReports.size());
       EXPECT_NE(0, breaked->m_CurrentCtx);
       client->abort();
   }

   TEST_F(LiveTest, breakpoint) {
       boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

       utils::receiveMessage<dglnet::message::Hello>(client.get(), getMessageHandler());
       dglnet::message::BreakedCall* breaked = utils::receiveMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
       EXPECT_EQ(wglGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
#else
       EXPECT_EQ(glXGetCurrentContext_Call, breaked->m_entryp.getEntrypoint());
#endif
       EXPECT_EQ(0, breaked->m_TraceSize);
       EXPECT_EQ(0, breaked->m_CtxReports.size());
       EXPECT_EQ(0, breaked->m_CurrentCtx);

       {
           std::set<Entrypoint> breakpoints;
           breakpoints.insert(glDrawArrays_Call);
           dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
           client->sendMessage(&breakPointMessage);
       }
       {
           dglnet::message::ContinueBreak continueMsg(false);
           client->sendMessage(&continueMsg);
       }

       breaked = utils::receiveMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);
        
       EXPECT_EQ(glDrawArrays_Call, breaked->m_entryp.getEntrypoint());

       client->abort();
   }

   TEST_F(LiveTest, entryp_retvals) {
       boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

       dglnet::message::BreakedCall* breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
#define _glCreateContext_Call wglCreateContext_Call
#else
#define _glCreateContext_Call glXCreateNewContext_Call
#endif
       {
           //set breakpoints && disable other breaking stuff
           std::set<Entrypoint> breakpoints;
           breakpoints.insert(_glCreateContext_Call);
           breakpoints.insert(glGetError_Call);
           breakpoints.insert(glCreateShader_Call);
           dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
           client->sendMessage(&breakPointMessage);

           dglnet::message::Configuration config(getUsualConfig());
           client->sendMessage(&config);
       }
       {
           //continue
           dglnet::message::ContinueBreak continueMsg(false);
           client->sendMessage(&continueMsg);
       }

       bool wglCreateContext_done = false;
       bool glGetError_done = false;
       bool glCreateShader_done = false;

       while (!wglCreateContext_done || !glGetError_done || !glCreateShader_done) {
           breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
           ASSERT_TRUE(breaked != NULL);

           {
               //we should be breaked on one of earlier breakpoints
               ASSERT_TRUE(
                   breaked->m_entryp.getEntrypoint() == _glCreateContext_Call ||
                   breaked->m_entryp.getEntrypoint() == glGetError_Call ||
                   breaked->m_entryp.getEntrypoint() == glCreateShader_Call
                   );
               
               {
                   //advance one call
                   dglnet::message::ContinueBreak step(dglnet::message::ContinueBreak::StepMode::CALL);
                   client->sendMessage(&step);
                   ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler()) != NULL);
               }
               
               {
                   //query callTrace with one entryp
                   dglnet::message::QueryCallTrace queryCallTrace(0, 1);
                   client->sendMessage(&queryCallTrace);
               }

               dglnet::message::CallTrace* callTrace = utils::receiveUntilMessage<dglnet::message::CallTrace>(client.get(), getMessageHandler());
               ASSERT_TRUE(callTrace != NULL);

#ifdef _WIN32
               HGLRC ctx;
#else
               GLXContext ctx;
#endif
               GLenum error;
               GLuint shader;

               switch (callTrace->m_Trace[0].getEntrypoint()) {
                   case _glCreateContext_Call:
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
               dglnet::message::ContinueBreak continueMsg(false);
               client->sendMessage(&continueMsg);
           }
       }

       client->abort();
   }

   TEST_F(LiveTest, framebuffer_query) {
       boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

       dglnet::message::BreakedCall* breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);

       {
           //disable breaking stuff
           dglnet::message::Configuration config(getUsualConfig());
           client->sendMessage(&config);
       }
      
       breaked = utils::runUntilEntryPoint(client,  getMessageHandler(), glDrawArrays_Call);

       for (int step = 0; step < 2; step++) {
           //step == 0: before first draw (GL_BACK should be cleared)
           //step == 1: after first draw (GL_BACK  should contain quad)
           {
               //query framebuffer
               dglnet::message::Request request(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Framebuffer, dglnet::ContextObjectName(breaked->m_CurrentCtx, GL_BACK)));
               client->sendMessage(&request);
           }

           dglnet::message::RequestReply* reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
           std::string nothing;
           ASSERT_TRUE(reply->isOk(nothing));
           dglnet::resource::DGLResourceFramebuffer * framebufferResource = dynamic_cast<dglnet::resource::DGLResourceFramebuffer*>(reply->m_Reply.get());
           ASSERT_TRUE(framebufferResource != NULL);

           ASSERT_EQ(GL_RGBA, framebufferResource->m_PixelRectangle->m_GLFormat);
           ASSERT_EQ(GL_UNSIGNED_BYTE, framebufferResource->m_PixelRectangle->m_GLType);
           EXPECT_EQ(0, framebufferResource->m_PixelRectangle->m_Samples);
           EXPECT_EQ(640, framebufferResource->m_PixelRectangle->m_Width);
           EXPECT_EQ(480, framebufferResource->m_PixelRectangle->m_Height);
           EXPECT_EQ(0, framebufferResource->m_PixelRectangle->m_InternalFormat);

           if (step) {
               utils::checkColor((GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
                   framebufferResource->m_PixelRectangle->m_Width,
                   framebufferResource->m_PixelRectangle->m_Height,
                   framebufferResource->m_PixelRectangle->m_RowBytes, 102, 127, 204, 255);
           } else {
               utils::checkColor((GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
                   framebufferResource->m_PixelRectangle->m_Width,
                   framebufferResource->m_PixelRectangle->m_Height,
                   framebufferResource->m_PixelRectangle->m_RowBytes, 0, 0, 0, 0);
           }
           
           //advance one call
           dglnet::message::ContinueBreak stepCall(dglnet::message::ContinueBreak::StepMode::CALL);
           client->sendMessage(&stepCall);
           ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler()) != NULL);
       }

       client->abort();
   }
    
   TEST_F(LiveTest, edit_shader) {
       boost::shared_ptr<dglnet::Client> client = getClientFor("simple.py");

       dglnet::message::BreakedCall* breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
       ASSERT_TRUE(breaked != NULL);

       {
           //disable other breaking stuff
           dglnet::message::Configuration config(getUsualConfig());
           client->sendMessage(&config);
       }

       breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glLinkProgram_Call);

       //we jutt before glLinkProgram(). Get the fragment shader

       ASSERT_EQ(1, breaked->m_CtxReports.size());
       ASSERT_EQ(2, breaked->m_CtxReports[0].m_ShaderSpace.size());

       gl_t fragId = 0;
       for (auto i = breaked->m_CtxReports[0].m_ShaderSpace.begin(); i !=  breaked->m_CtxReports[0].m_ShaderSpace.end(); i++) {
           if (i->m_Target == GL_FRAGMENT_SHADER) {
               fragId = i->m_Name;
           }
       }

       {
           dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, fragId)));
           client->sendMessage(&requestMessage);
       }

       dglnet::message::RequestReply  * reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       std::string nothing;
       ASSERT_TRUE(reply->isOk(nothing));
       dglnet::resource::DGLResourceShader * shaderResource = dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get());
       
       //Edit frag shader
       std::string oldSource = shaderResource->m_Source, source = oldSource;
       std::string oldStr = "vec4(0.4, 0.5, 0.8, 1.0)";
       std::string newStr = "vec4(0.8, 0.5, 0.4, 1.0)";
       size_t pos = 0;
       while((pos = source.find(oldStr, pos)) != std::string::npos) {
           source.replace(pos, oldStr.length(), newStr);
           pos += newStr.length();
       }
       

       dglnet::message::Request requestMessage(new dglnet::request::EditShaderSource(breaked->m_CurrentCtx, fragId, false, source));
       client->sendMessage(&requestMessage);
       reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       ASSERT_TRUE(reply->isOk(nothing));

       //Verify frag shader
       {
           dglnet::message::Request req(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, fragId)));
           client->sendMessage(&req);
       }
       reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       ASSERT_TRUE(reply->isOk(nothing));
       shaderResource = dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get());
       ASSERT_EQ(source, shaderResource->m_Source);
       ASSERT_EQ(shaderResource->m_CompileStatus.second, GL_TRUE);

       //Verify by drawing with edited shader
       {
           dglnet::message::ContinueBreak stepDrawCall(dglnet::message::ContinueBreak::StepMode::FRAME);
           client->sendMessage(&stepDrawCall);
           ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler()) != NULL);
       }

       //Query back framebuffer
       {
           dglnet::message::Request req(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Framebuffer, dglnet::ContextObjectName(breaked->m_CurrentCtx, GL_BACK)));
           client->sendMessage(&req);
       }
       reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       ASSERT_TRUE(reply->isOk(nothing));
       dglnet::resource::DGLResourceFramebuffer* framebufferResource = dynamic_cast<dglnet::resource::DGLResourceFramebuffer*>(reply->m_Reply.get());
       ASSERT_TRUE(framebufferResource != NULL);

       utils::checkColor((GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
               framebufferResource->m_PixelRectangle->m_Width,
               framebufferResource->m_PixelRectangle->m_Height,
               framebufferResource->m_PixelRectangle->m_RowBytes, 204, 127, 102, 255);


       //Reset shader to default
       {
           dglnet::message::Request req(new dglnet::request::EditShaderSource(breaked->m_CurrentCtx, fragId, true));
           client->sendMessage(&req);
       }
       reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       ASSERT_TRUE(reply->isOk(nothing));

       //Verify frag shader
       {
           dglnet::message::Request req(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, fragId)));
           client->sendMessage(&req);
       }
       reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
       ASSERT_TRUE(reply->isOk(nothing));
       shaderResource = dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get());
       ASSERT_EQ(oldSource, shaderResource->m_Source);
       ASSERT_EQ(shaderResource->m_CompileStatus.second, GL_TRUE);

       client->abort();
   }


   TEST_F(LiveTest, shader_handling) {
        dglnet::message::RequestReply* reply;
        std::string nothing;
        GLuint shaderId, programId;

        boost::shared_ptr<dglnet::Client> client = getClientFor("shader_handling.py");
        
        dglnet::message::BreakedCall* breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(client.get(), getMessageHandler());
        ASSERT_TRUE(breaked != NULL);
        
        {
            //disable other breaking stuff
            dglnet::message::Configuration config(getUsualConfig());
            client->sendMessage(&config);
        }

        //#simple case: create-delete
        //shader = glCreateShader(GL_VERTEX_SHADER);
        //#test point: we have one shader
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDeleteShader_Call);
        EXPECT_EQ(1, breaked->m_CtxReports[0].m_ShaderSpace.size());
        EXPECT_EQ(GL_VERTEX_SHADER, breaked->m_CtxReports[0].m_ShaderSpace.begin()->m_Target);
        //glDeleteShader(shader);
        //#test point: shader deleted
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glFlush_Call);
        EXPECT_EQ(1, breaked->m_CtxReports[0].m_ShaderSpace.size());
        shaderId = (GLuint)breaked->m_CtxReports[0].m_ShaderSpace.begin()->m_Name;
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        //glFlush(); #flush is only to mark case end


	    //#source cache test
	    //shader = glCreateShader(GL_VERTEX_SHADER);
	    //glShaderSource(shader, source);
	    //glDeleteShader(shader);
	    //#test point: shader deleted, has cached source
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glFlush_Call);
        if (breaked->m_CtxReports[0].m_ShaderSpace.size() == 1) {
            //same id for new shader
            shaderId = (GLuint)breaked->m_CtxReports[0].m_ShaderSpace.begin()->m_Name;
        } else if (breaked->m_CtxReports[0].m_ShaderSpace.size() == 2) {
            //new id for new shader, look up the list
            auto i = breaked->m_CtxReports[0].m_ShaderSpace.begin();
            if (shaderId != i->m_Name) shaderId = (GLuint)i->m_Name; else  shaderId = (GLuint)(++i)->m_Name;
        } else { ASSERT_TRUE(0); }
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_Source.find("gl_Position") != std::string::npos);
        //glFlush(); #flush is only to mark case end
        
		
        //#lazy deletion test, create-attach-delete-detach
        //shader = glCreateShader(GL_VERTEX_SHADER);
        //program = glCreateProgram();
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glAttachShader_Call);
        breaked->m_entryp.getArgs()[0].get(programId);
        breaked->m_entryp.getArgs()[1].get(shaderId);
        //glAttachShader(program, shader);
        //#test point: program has shader
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDeleteShader_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Program, dglnet::ContextObjectName(breaked->m_CurrentCtx, programId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_EQ(1, dynamic_cast<dglnet::resource::DGLResourceProgram*>(reply->m_Reply.get())->m_AttachedShaders.size());
        EXPECT_EQ(shaderId, dynamic_cast<dglnet::resource::DGLResourceProgram*>(reply->m_Reply.get())->m_AttachedShaders[0].first);
        //glDeleteShader(shader);
        //#test point: shader not deleted
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDetachShader_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_FALSE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        //glDetachShader(program, shader);
        //#test point: shader deleted, program has no shader
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDeleteProgram_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Program, dglnet::ContextObjectName(breaked->m_CurrentCtx, programId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_EQ(0, dynamic_cast<dglnet::resource::DGLResourceProgram*>(reply->m_Reply.get())->m_AttachedShaders.size());
        //glDeleteProgram(program)
		//glFlush(); #flush is only to mark case end


        //#lazy deletion test2: create-attach-delete-deleteprogram
        //shader = glCreateShader(GL_VERTEX_SHADER);
        //program = glCreateProgram();
        //glAttachShader(program, shader);
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glAttachShader_Call);
        breaked->m_entryp.getArgs()[0].get(programId);
        breaked->m_entryp.getArgs()[1].get(shaderId);
        //glDeleteShader(shader);
        //#test point: shader not deleted, program has shader
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDeleteProgram_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_FALSE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Program, dglnet::ContextObjectName(breaked->m_CurrentCtx, programId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_EQ(1, dynamic_cast<dglnet::resource::DGLResourceProgram*>(reply->m_Reply.get())->m_AttachedShaders.size());
        EXPECT_EQ(shaderId, dynamic_cast<dglnet::resource::DGLResourceProgram*>(reply->m_Reply.get())->m_AttachedShaders[0].first);
        //glDeleteProgram(program)
        //#test point: shader deleted
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glFlush_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        //glFlush(); #flush is only to mark case end
		


        //#caching in lazy deletion test (source after delete by detach) 
        //shader = glCreateShader(GL_VERTEX_SHADER);
        //program = glCreateProgram();
        //glAttachShader(program, shader);
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glAttachShader_Call);
        breaked->m_entryp.getArgs()[0].get(programId);
        breaked->m_entryp.getArgs()[1].get(shaderId);
        //glDeleteShader(shader);
        //glShaderSource(shader, source);
        //glDetachShader(program, shader);
        //#test point: cached source
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glDeleteProgram_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_Source.find("gl_Position") != std::string::npos);
        //glDeleteProgram(program)
        //glFlush(); #flush is only to mark case end
	


        //#caching in lazy deletion test 2 (source after delete by deleteprogram) 
        //shader = glCreateShader(GL_VERTEX_SHADER);
        //program = glCreateProgram();
        //glAttachShader(program, shader);
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glAttachShader_Call);
        breaked->m_entryp.getArgs()[0].get(programId);
        breaked->m_entryp.getArgs()[1].get(shaderId);
        //glDeleteShader(shader);
        //glShaderSource(shader, source);
        //glDeleteProgram(program)
        //#test point: cached source
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(), glFlush_Call);
        {
            dglnet::message::Request requestMessage(new dglnet::request::QueryResource(dglnet::DGLResource::ObjectType::Shader, dglnet::ContextObjectName(breaked->m_CurrentCtx, shaderId)));
            client->sendMessage(&requestMessage);
        }
        reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(client.get(), getMessageHandler());
        ASSERT_TRUE(reply->isOk(nothing));
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_ShaderObjDeleted);
        EXPECT_TRUE(dynamic_cast<dglnet::resource::DGLResourceShader*>(reply->m_Reply.get())->m_Source.find("gl_Position") != std::string::npos);
        //glFlush(); #flush is only to mark case end
   }



}  // namespace



