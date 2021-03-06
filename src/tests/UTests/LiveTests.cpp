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

#include <DGLGui/dglprocess.h>

#include "gtest/gtest.h"
#include <DGLNet/client.h>
#include <DGLCommon/gl-headers.h>
#include <DGLCommon/gl-glue-headers.h>

#include <DGLNet/protocol/message.h>
#include <DGLNet/protocol/request.h>
#include <DGLNet/protocol/resource.h>
#include <DGLNet/protocol/messagehandler.h>
#include <DGLNet/protocol/dglconfiguration.h>


#include <thread>
#include <chrono>

#include "LiveProcessWrapper.h"

namespace {

class LiveTest : public ::testing::Test {
   protected:
    LiveTest() {
        if (once) {
            once = false;
        }
    }

    virtual ~LiveTest() {}

    class Controller : public dglnet::IController {
       public:
        virtual void onSetStatus(std::string) {}
        virtual void onSocket() {}
        virtual void onSocketStartSend() {}
        virtual void onSocketStopSend() {}
    } m_Controller;

   public:
    class MessageHandler : public dglnet::MessageHandler {
#define MAKE_HANDLER(T)                                                \
    virtual void doHandle##T(const dglnet::message::T& msg) override { \
        mLastMessage = new dglnet::message::T(msg);                    \
    }

        MAKE_HANDLER(Hello);
        MAKE_HANDLER(Configuration);
        MAKE_HANDLER(BreakedCall);
        MAKE_HANDLER(ContinueBreak);
        MAKE_HANDLER(QueryCallTrace);
        MAKE_HANDLER(CallTrace);
        MAKE_HANDLER(Request);
        MAKE_HANDLER(RequestReply);
        MAKE_HANDLER(SetBreakPoints);
        void doHandleConnect() {}
        void doHandleDisconnect(const std::string& msg) {
            mDisconnected = true;
            mDisconnectedReason = msg;
        }

       public:
        MessageHandler() : mLastMessage(NULL), mDisconnected(false) {};
        dglnet::Message* getLastMessage() {
            dglnet::Message* ret = mLastMessage;
            mLastMessage = NULL;
            return ret;
        }

        dglnet::Message* mLastMessage;
        bool mDisconnected;
        std::string mDisconnectedReason;
    };

   private:
    MessageHandler m_MessageHandler;

   public:
    MessageHandler& getMessageHandler() { return m_MessageHandler; }

    std::shared_ptr<dglnet::Client> getClientFor(std::string sampleName) {

        m_ProcessWrapper = std::make_shared<LiveProcessWrapper>(sampleName);
        
        std::shared_ptr<dglnet::Client> client =
            dglnet::Client::Create(&m_Controller, &m_MessageHandler);

        client->connectServer("127.0.0.1", "8888");

        return client;
    }

    std::shared_ptr<dglnet::Client> getAnotherConnection() {

        m_ProcessWrapper->waitForSocket();

        std::shared_ptr<dglnet::Client> client =
            dglnet::Client::Create(&m_Controller, &m_MessageHandler);

        client->connectServer("127.0.0.1", "8888");
        return client;
    }

    void terminate(std::shared_ptr<dglnet::Client> client) {
        dglnet::message::Terminate msg;
        client->sendMessage(&msg);
        while (client->run_one()) {}
        client->abort();
    }

    DGLConfiguration getUsualConfig() {
        DGLConfiguration config;
        config.m_BreakOnGLError = false;
        config.m_BreakOnDebugOutput = false;
        config.m_BreakOnCompilerError = false;
        return config;
    }

    static bool once;

    std::shared_ptr<LiveProcessWrapper> m_ProcessWrapper;
};
bool LiveTest::once = true;

namespace utils {
template <typename T>
T* receiveMessage(dglnet::ITransport* transport,
                  LiveTest::MessageHandler& handler) {
    dglnet::Message* msg = NULL;
    do {
        transport->run_one();
    } while (!handler.mDisconnected &&
             !((msg = handler.getLastMessage()) != NULL));
    EXPECT_EQ(false, handler.mDisconnected);
    if (!msg || !dynamic_cast<T*>(msg)) {
        return NULL;
    }
    return dynamic_cast<T*>(msg);
}

template <typename T>
T* receiveUntilMessage(dglnet::ITransport* transport,
                       LiveTest::MessageHandler& handler) {
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

dglnet::message::BreakedCall* runUntilEntryPoint(
        std::shared_ptr<dglnet::Client> client,
        LiveTest::MessageHandler& handler, Entrypoint entryp) {
    // set break point
    {
        std::set<Entrypoint> breakpoints;
        breakpoints.insert(entryp);
        dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
        client->sendMessage(&breakPointMessage);
    }

    // continue
    {
        dglnet::message::ContinueBreak continueMsg(false);
        client->sendMessage(&continueMsg);
    }

    // wait for break
    dglnet::message::BreakedCall* ret =
            utils::receiveMessage<dglnet::message::BreakedCall>(client.get(),
                                                                handler);

    // clear breakpoints
    {
        std::set<Entrypoint> breakpoints;
        dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
        client->sendMessage(&breakPointMessage);
    }

    EXPECT_TRUE(ret != NULL);
    EXPECT_EQ(entryp, ret->m_entryp.getEntrypoint());

    return ret;
}

void checkColor(GLubyte* ptr, int width, int height, int rowBytes, int r, int g,
                int b, int a) {
    for (int y = 0; y < height; y++) {
        GLubyte* rowPtr = y * rowBytes + ptr;
        for (int x = 0; x < width; x++) {
            ASSERT_TRUE(abs(r - rowPtr[4 * x + 0]) <= 1);
            ASSERT_TRUE(abs(g - rowPtr[4 * x + 1]) <= 1);
            ASSERT_TRUE(abs(b - rowPtr[4 * x + 2]) <= 1);
            ASSERT_TRUE(abs(a - rowPtr[4 * x + 3]) <= 1);
        }
    }
}

void checkColorRect(GLubyte* ptr, int width, int height, int rowBytes, int r,
                    int g, int b, int a) {
    for (int y = 0; y < height; y++) {
        GLubyte* rowPtr = y * rowBytes + ptr;
        for (int x = 0; x < width; x++) {
            if (x > 0.3 * width && x < 0.7 * width && y > 0.3 * height &&
                y < 0.7 * height) {
                ASSERT_TRUE(abs(r - rowPtr[4 * x + 0]) <= 1);
                ASSERT_TRUE(abs(g - rowPtr[4 * x + 1]) <= 1);
                ASSERT_TRUE(abs(b - rowPtr[4 * x + 2]) <= 1);
                ASSERT_TRUE(abs(a - rowPtr[4 * x + 3]) <= 1);
            }
            if (x < 0.2 * width || x > 0.8 * width || y < 0.2 * height ||
                y > 0.8 * height) {
                if (abs(rowPtr[4 * x + 0]) > 0) {
                    printf("%d %d %d %d\n", rowPtr[4 * x + 0],
                           rowPtr[4 * x + 1], rowPtr[4 * x + 2],
                           rowPtr[4 * x + 3]);
                    fflush(stdout);
                }
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

    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::Hello* hello =
            utils::receiveMessage<dglnet::message::Hello>(client.get(),
                                                          getMessageHandler());
    ASSERT_TRUE(hello != NULL);
#ifdef _WIN32
    EXPECT_EQ("samples.exe", hello->m_ProcessName);
#else
    EXPECT_TRUE(std::string::npos != hello->m_ProcessName.find("samples"));
#endif
    terminate(client);
    EXPECT_TRUE(client.unique());
}

TEST_F(LiveTest, multiple_connect_disconnect) {

    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::Hello* hello =
        utils::receiveMessage<dglnet::message::Hello>(client.get(),
        getMessageHandler());
    ASSERT_TRUE(hello != NULL);
#ifdef _WIN32
    EXPECT_EQ("samples.exe", hello->m_ProcessName);
#else
    EXPECT_TRUE(std::string::npos != hello->m_ProcessName.find("samples"));
#endif
    terminate(client);
    EXPECT_TRUE(client.unique());
}

TEST_F(LiveTest, connect_disconnect_reconnect) {

    //connect
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::Hello* hello =
        utils::receiveMessage<dglnet::message::Hello>(client.get(),
        getMessageHandler());
    ASSERT_TRUE(hello != NULL);

    dglnet::message::BreakedCall* breaked =
        utils::receiveMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    for (int i=0; i < 10; i++) {
        client->abort();
        client = getAnotherConnection();

        dglnet::message::Hello* hello2 =
            utils::receiveMessage<dglnet::message::Hello>(client.get(),
            getMessageHandler());
        ASSERT_TRUE(hello2 != NULL);

        dglnet::message::BreakedCall* breaked2 =
            utils::receiveMessage<dglnet::message::BreakedCall>(
            client.get(), getMessageHandler());
        ASSERT_TRUE(breaked2 != NULL);

    }
    terminate(client);
}

TEST_F(LiveTest, continue_break) {
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    utils::receiveMessage<dglnet::message::Hello>(client.get(),
                                                  getMessageHandler());
    dglnet::message::BreakedCall* breaked =
            utils::receiveMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
    EXPECT_EQ(DescribePixelFormat_Call, breaked->m_entryp.getEntrypoint());
#else
    EXPECT_EQ(glXQueryExtension_Call, breaked->m_entryp.getEntrypoint());
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

        breaked = utils::receiveMessage<dglnet::message::BreakedCall>(
                client.get(), getMessageHandler());
        ASSERT_TRUE(breaked != NULL);
    } while (!breaked->m_CurrentCtx);
    EXPECT_NE(0, breaked->m_TraceSize);
    EXPECT_EQ(1, breaked->m_CtxReports.size());
    EXPECT_NE(0, breaked->m_CurrentCtx);
    terminate(client);
}

TEST_F(LiveTest, breakpoint) {
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    utils::receiveMessage<dglnet::message::Hello>(client.get(),
                                                  getMessageHandler());
    dglnet::message::BreakedCall* breaked =
            utils::receiveMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
    EXPECT_EQ(DescribePixelFormat_Call, breaked->m_entryp.getEntrypoint());
#else
    EXPECT_EQ(glXQueryExtension_Call, breaked->m_entryp.getEntrypoint());
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

    breaked = utils::receiveMessage<dglnet::message::BreakedCall>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    EXPECT_EQ(glDrawArrays_Call, breaked->m_entryp.getEntrypoint());

    terminate(client);
}

TEST_F(LiveTest, entryp_retvals) {
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

#ifdef _WIN32
#define _glMakeCurrent_Call wglMakeCurrent_Call
#else
#define _glMakeCurrent_Call glXMakeCurrent_Call
#endif
    {
        // set breakpoints && disable other breaking stuff
        std::set<Entrypoint> breakpoints;
        breakpoints.insert(_glMakeCurrent_Call);
        breakpoints.insert(glGetError_Call);
        breakpoints.insert(glCreateShader_Call);
        dglnet::message::SetBreakPoints breakPointMessage(breakpoints);
        client->sendMessage(&breakPointMessage);

        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }
    {
        // continue
        dglnet::message::ContinueBreak continueMsg(false);
        client->sendMessage(&continueMsg);
    }

    bool wglMakeCurrent_done = false;
    bool glGetError_done = false;
    bool glCreateShader_done = false;

    while (!wglMakeCurrent_done || !glGetError_done || !glCreateShader_done) {
        breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                client.get(), getMessageHandler());
        ASSERT_TRUE(breaked != NULL);

        {
            // we should be breaked on one of earlier breakpoints
            ASSERT_TRUE(
                    breaked->m_entryp.getEntrypoint() == _glMakeCurrent_Call ||
                    breaked->m_entryp.getEntrypoint() == glGetError_Call ||
                    breaked->m_entryp.getEntrypoint() == glCreateShader_Call);

            {
                // advance one call
                dglnet::message::ContinueBreak step(
                        dglnet::message::StepMode::CALL);
                client->sendMessage(&step);
                ASSERT_TRUE(utils::receiveUntilMessage<
                                    dglnet::message::BreakedCall>(
                                    client.get(), getMessageHandler()) != NULL);
            }

            {
                // query callTrace with one entryp
                dglnet::message::QueryCallTrace queryCallTrace(0, 1);
                client->sendMessage(&queryCallTrace);
            }

            dglnet::message::CallTrace* callTrace =
                    utils::receiveUntilMessage<dglnet::message::CallTrace>(
                            client.get(), getMessageHandler());
            ASSERT_TRUE(callTrace != NULL);

#ifdef _WIN32
            BOOL ctxStatus;
#else
            Bool ctxStatus;
#endif
            GLenum error;
            GLuint shader;

            switch (callTrace->m_Trace[0].getEntrypoint()) {
                case _glMakeCurrent_Call:
                    wglMakeCurrent_done = true;
                    callTrace->m_Trace[0].getRetVal().get(ctxStatus);
                    EXPECT_TRUE(ctxStatus == TRUE);
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

            // continue execution
            dglnet::message::ContinueBreak continueMsg(false);
            client->sendMessage(&continueMsg);
        }
    }

    terminate(client);
}

TEST_F(LiveTest, framebuffer_query) {
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    // skip first frame (rendering broken on some implementations)
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDrawArrays_Call);
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glClear_Call);

    // break just before drawcall
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDrawArrays_Call);

    for (int step = 0; step < 2; step++) {
        // step == 0: before first draw (GL_BACK should be cleared)
        // step == 1: after first draw (GL_BACK  should contain quad)
        {
            // query framebuffer
            dglnet::message::Request request(new dglnet::request::QueryResource(
                    dglnet::message::ObjectType::Framebuffer,
                    dglnet::ContextObjectName(breaked->m_CurrentCtx, GL_BACK)));
            client->sendMessage(&request);
        }

        dglnet::message::RequestReply* reply =
                utils::receiveUntilMessage<dglnet::message::RequestReply>(
                        client.get(), getMessageHandler());
        std::string nothing;
        ASSERT_TRUE(reply->isOk(nothing));
        dglnet::resource::DGLResourceFramebuffer* framebufferResource =
                dynamic_cast<dglnet::resource::DGLResourceFramebuffer*>(
                        reply->m_Reply.get());
        ASSERT_TRUE(framebufferResource != NULL);

        ASSERT_EQ(GL_RGBA, framebufferResource->m_PixelRectangle->m_GLFormat);
        ASSERT_EQ(GL_UNSIGNED_BYTE,
                  framebufferResource->m_PixelRectangle->m_GLType);

        EXPECT_EQ(640, framebufferResource->m_PixelRectangle->m_Width);
        EXPECT_EQ(480, framebufferResource->m_PixelRectangle->m_Height);

        if (step) {
            utils::checkColorRect(
                    (GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
                    framebufferResource->m_PixelRectangle->m_Width,
                    framebufferResource->m_PixelRectangle->m_Height,
                    framebufferResource->m_PixelRectangle->m_RowBytes, 102, 127,
                    204, 255);
        } else {
            utils::checkColorRect(
                    (GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
                    framebufferResource->m_PixelRectangle->m_Width,
                    framebufferResource->m_PixelRectangle->m_Height,
                    framebufferResource->m_PixelRectangle->m_RowBytes, 0, 0, 0,
                    0);
        }

        // advance one call
        dglnet::message::ContinueBreak stepCall(
                dglnet::message::StepMode::CALL);
        client->sendMessage(&stepCall);
        ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                            client.get(), getMessageHandler()) != NULL);
    }

    terminate(client);
}

TEST_F(LiveTest, framebuffer_resize) {
    std::shared_ptr<dglnet::Client> client = getClientFor("resize");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    // skip first frame (rendering broken on some implementations)
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDrawArrays_Call);
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glClear_Call);

    for (int step = 0; step < 2; step++) {
        // step == 0: frame 2
        // step == 1: frame 3

        breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                            glDrawArrays_Call);
        // advance one call
        dglnet::message::ContinueBreak stepCall(
                dglnet::message::StepMode::CALL);
        client->sendMessage(&stepCall);
        ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                            client.get(), getMessageHandler()) != NULL);

        {
            // query framebuffer
            dglnet::message::Request request(new dglnet::request::QueryResource(
                    dglnet::message::ObjectType::Framebuffer,
                    dglnet::ContextObjectName(breaked->m_CurrentCtx, GL_BACK)));
            client->sendMessage(&request);
        }

        dglnet::message::RequestReply* reply =
                utils::receiveUntilMessage<dglnet::message::RequestReply>(
                        client.get(), getMessageHandler());
        std::string nothing;
        ASSERT_TRUE(reply->isOk(nothing));
        dglnet::resource::DGLResourceFramebuffer* framebufferResource =
                dynamic_cast<dglnet::resource::DGLResourceFramebuffer*>(
                        reply->m_Reply.get());
        ASSERT_TRUE(framebufferResource != NULL);

        int expectedSize = step == 0 ? 200 : 400;

        EXPECT_EQ(expectedSize, framebufferResource->m_PixelRectangle->m_Width);
        EXPECT_EQ(expectedSize,
                  framebufferResource->m_PixelRectangle->m_Height);

        utils::checkColorRect(
                (GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
                framebufferResource->m_PixelRectangle->m_Width,
                framebufferResource->m_PixelRectangle->m_Height,
                framebufferResource->m_PixelRectangle->m_RowBytes, 102, 127,
                204, 255);
    }
    terminate(client);
}

TEST_F(LiveTest, texture_query_2d) {
    std::shared_ptr<dglnet::Client> client = getClientFor("texture2d");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDrawArrays_Call);

    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_TextureSpace.size());

    EXPECT_EQ(GL_TEXTURE_2D,
              breaked->m_CtxReports[0].m_TextureSpace.begin()->m_Target);

    {
        // query texture
        dglnet::message::Request request(new dglnet::request::QueryResource(
                dglnet::message::ObjectType::Texture,
                dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                          breaked->m_CtxReports[0]
                                                  .m_TextureSpace.begin()
                                                  ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
            utils::receiveUntilMessage<dglnet::message::RequestReply>(
                    client.get(), getMessageHandler());
    std::string nothing;
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceTexture* textureResource =
            dynamic_cast<dglnet::resource::DGLResourceTexture*>(
                    reply->m_Reply.get());
    ASSERT_TRUE(textureResource != NULL);

    ASSERT_EQ(GL_TEXTURE_2D, textureResource->m_Target);

    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers.size());
    ASSERT_EQ(5, textureResource->m_FacesLevelsLayers[0].size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0][0].size());

    int size = 16;

    GLubyte colors[5][4] = {
        {102, 127, 204, 255},
        { 140, 32, 48, 223}, 
        { 74, 189, 232, 239}, 
        { 214, 72, 239, 87},
        { 144, 223, 142, 223 },
    };

    
    for (size_t i = 0; i < textureResource->m_FacesLevelsLayers[0].size(); i++) {

        if (i > 4) break;

        dglnet::resource::DGLResourceTexture::TextureLayer& layer = 
            textureResource->m_FacesLevelsLayers[0][i][0];

        dglnet::resource::DGLPixelRectangle* rect =
            layer.m_PixelRectangle.get();

        EXPECT_TRUE(layer.m_Samples == 0 || layer.m_Samples == 1);
        EXPECT_EQ(GL_RGBA8, layer.m_InternalFormat);

        ASSERT_EQ(GL_RGBA, rect->m_GLFormat);
        ASSERT_EQ(GL_UNSIGNED_BYTE, rect->m_GLType);
        EXPECT_EQ(size, rect->m_Width);
        EXPECT_EQ((size + 1) / 2, rect->m_Height);
        
        utils::checkColor((GLubyte*)rect->getPtr(), rect->m_Width, rect->m_Height,
            rect->m_RowBytes, colors[i][0], colors[i][1], colors[i][2], colors[i][3]);

        size /= 2;
    }
    terminate(client);
}


TEST_F(LiveTest, texture_query_3d) {
    std::shared_ptr<dglnet::Client> client = getClientFor("texture3d");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glDrawArrays_Call);

    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_TextureSpace.size());

    EXPECT_EQ(GL_TEXTURE_3D,
        breaked->m_CtxReports[0].m_TextureSpace.begin()->m_Target);

    {
        // query texture
        dglnet::message::Request request(new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Texture,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            breaked->m_CtxReports[0]
        .m_TextureSpace.begin()
            ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
        utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    std::string nothing;
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceTexture* textureResource =
        dynamic_cast<dglnet::resource::DGLResourceTexture*>(
        reply->m_Reply.get());
    ASSERT_TRUE(textureResource != NULL);


    ASSERT_EQ(GL_TEXTURE_3D, textureResource->m_Target);

    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers.size());
    ASSERT_EQ(5, textureResource->m_FacesLevelsLayers[0].size());
    ASSERT_EQ(4, textureResource->m_FacesLevelsLayers[0][0].size());
    ASSERT_EQ(2, textureResource->m_FacesLevelsLayers[0][1].size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0][2].size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0][3].size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0][4].size());

    int size = 16;

    GLubyte colors[5][4] = {
        {102, 127, 204, 255},
        { 140, 32, 48, 223}, 
        { 74, 189, 232, 239}, 
        { 214, 72, 239, 87},
        { 144, 223, 142, 223 },
    };

    for (size_t i = 0; i < textureResource->m_FacesLevelsLayers[0].size(); i++) {

        if (i > 4) break;

        dglnet::resource::DGLResourceTexture::TextureLayer& layer = 
            textureResource->m_FacesLevelsLayers[0][i][0];

        dglnet::resource::DGLPixelRectangle* rect =
            layer.m_PixelRectangle.get();

        EXPECT_TRUE(layer.m_Samples == 0 || layer.m_Samples == 1);
        EXPECT_EQ(GL_RGBA8, layer.m_InternalFormat);
        ASSERT_EQ(GL_RGBA, rect->m_GLFormat);
        ASSERT_EQ(GL_UNSIGNED_BYTE, rect->m_GLType);
        EXPECT_EQ(size, rect->m_Width);
        EXPECT_EQ((size + 1) / 2, rect->m_Height);


        utils::checkColor((GLubyte*)rect->getPtr(), rect->m_Width, rect->m_Height,
            rect->m_RowBytes, colors[i][0], colors[i][1], colors[i][2], colors[i][3]);

        size /= 2;
    }
    terminate(client);
}

TEST_F(LiveTest, texture_query_2d_msaa) {
    std::shared_ptr<dglnet::Client> client = getClientFor("texture2d_msaa");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glDrawArrays_Call);

    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_TextureSpace.size());

    EXPECT_EQ(GL_TEXTURE_2D_MULTISAMPLE,
        breaked->m_CtxReports[0].m_TextureSpace.begin()->m_Target);

    {
        // query texture
        dglnet::message::Request request(new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Texture,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            breaked->m_CtxReports[0]
        .m_TextureSpace.begin()
            ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
        utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    std::string nothing;
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceTexture* textureResource =
        dynamic_cast<dglnet::resource::DGLResourceTexture*>(
        reply->m_Reply.get());
    ASSERT_TRUE(textureResource != NULL);

    ASSERT_EQ(GL_TEXTURE_2D_MULTISAMPLE, textureResource->m_Target);

    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers.size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0].size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0][0].size());

    GLubyte color[4] = {102, 127, 204, 255};
        
    dglnet::resource::DGLResourceTexture::TextureLayer& layer = 
        textureResource->m_FacesLevelsLayers[0][0][0];

    dglnet::resource::DGLPixelRectangle* rect =
        layer.m_PixelRectangle.get();

    EXPECT_EQ(4, layer.m_Samples);
    EXPECT_EQ(GL_RGBA8, layer.m_InternalFormat);

    ASSERT_EQ(GL_RGBA, rect->m_GLFormat);
    ASSERT_EQ(GL_UNSIGNED_BYTE, rect->m_GLType);
    EXPECT_EQ(16, rect->m_Width);
    EXPECT_EQ(32, rect->m_Height);

    utils::checkColor((GLubyte*)rect->getPtr(), rect->m_Width, rect->m_Height,
        rect->m_RowBytes, color[0], color[1], color[2], color[3]);

    terminate(client);
}

TEST_F(LiveTest, texture_query_2d_array_msaa) {
    std::shared_ptr<dglnet::Client> client = getClientFor("texture2d_array_msaa");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glDrawArrays_Call);

    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_TextureSpace.size());

    EXPECT_EQ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
        breaked->m_CtxReports[0].m_TextureSpace.begin()->m_Target);

    {
        // query texture
        dglnet::message::Request request(new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Texture,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            breaked->m_CtxReports[0]
        .m_TextureSpace.begin()
            ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
        utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    std::string nothing;
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceTexture* textureResource =
        dynamic_cast<dglnet::resource::DGLResourceTexture*>(
        reply->m_Reply.get());
    ASSERT_TRUE(textureResource != NULL);

    ASSERT_EQ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, textureResource->m_Target);

    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers.size());
    ASSERT_EQ(1, textureResource->m_FacesLevelsLayers[0].size());
    ASSERT_EQ(5, textureResource->m_FacesLevelsLayers[0][0].size());

    GLubyte colors[5][4] = {
        {102, 127, 204, 255},
        { 140, 32, 48, 223}, 
        { 74, 189, 232, 239}, 
        { 214, 72, 239, 87},
        { 144, 223, 142, 223 },
    };

    for (size_t i =0; i < textureResource->m_FacesLevelsLayers[0][0].size(); i++) {
        dglnet::resource::DGLResourceTexture::TextureLayer& layer = 
            textureResource->m_FacesLevelsLayers[0][0][i];

        dglnet::resource::DGLPixelRectangle* rect =
            layer.m_PixelRectangle.get();

        EXPECT_EQ(4, layer.m_Samples);
        EXPECT_EQ(GL_RGBA8, layer.m_InternalFormat);

        ASSERT_EQ(GL_RGBA, rect->m_GLFormat);
        ASSERT_EQ(GL_UNSIGNED_BYTE, rect->m_GLType);
        EXPECT_EQ(16, rect->m_Width);
        EXPECT_EQ(32, rect->m_Height);

        utils::checkColor((GLubyte*)rect->getPtr(), rect->m_Width, rect->m_Height,
            rect->m_RowBytes, colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
    }

    terminate(client);
}


TEST_F(LiveTest, edit_shader) {
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable other breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glLinkProgram_Call);

    // we jutt before glLinkProgram(). Get the fragment shader

    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(2, breaked->m_CtxReports[0].m_ShaderSpace.size());

    gl_t fragId = 0;
    for (auto i = breaked->m_CtxReports[0].m_ShaderSpace.begin();
         i != breaked->m_CtxReports[0].m_ShaderSpace.end(); i++) {
        if (i->m_Target == GL_FRAGMENT_SHADER) {
            fragId = i->m_Name;
        }
    }

    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Shader,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  fragId)));
        client->sendMessage(&requestMessage);
    }

    dglnet::message::RequestReply* reply =
            utils::receiveUntilMessage<dglnet::message::RequestReply>(
                    client.get(), getMessageHandler());
    std::string nothing;
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceShader* shaderResource =
            dynamic_cast<dglnet::resource::DGLResourceShader*>(
                    reply->m_Reply.get());

    // Edit frag shader
    std::string oldSource = shaderResource->m_Source, source = oldSource;
    std::string oldStr = "vec4(0.4, 0.5, 0.8, 1.0)";
    std::string newStr = "vec4(0.8, 0.5, 0.4, 1.0)";
    size_t pos = 0;
    while ((pos = source.find(oldStr, pos)) != std::string::npos) {
        source.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

    dglnet::message::Request requestMessage(
            new dglnet::request::EditShaderSource(breaked->m_CurrentCtx, fragId,
                                                  false, source));
    client->sendMessage(&requestMessage);
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));

    // Verify frag shader
    {
        dglnet::message::Request req(new dglnet::request::QueryResource(
                dglnet::message::ObjectType::Shader,
                dglnet::ContextObjectName(breaked->m_CurrentCtx, fragId)));
        client->sendMessage(&req);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    shaderResource = dynamic_cast<dglnet::resource::DGLResourceShader*>(
            reply->m_Reply.get());
    ASSERT_EQ(source, shaderResource->m_Source);
    ASSERT_EQ(shaderResource->m_CompileStatus.second, GL_TRUE);

    // Skip few frames (first frames rendering broken on some implementations).
    for (int i = 0; i < 4; i++) {
        dglnet::message::ContinueBreak stepDrawCall(
                dglnet::message::StepMode::FRAME);
        client->sendMessage(&stepDrawCall);
        ASSERT_TRUE(utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                            client.get(), getMessageHandler()) != NULL);
    }

    // Verify by drawing with edited shader
    // Query back framebuffer
    {
        dglnet::message::Request req(new dglnet::request::QueryResource(
                dglnet::message::ObjectType::Framebuffer,
                dglnet::ContextObjectName(breaked->m_CurrentCtx, GL_BACK)));
        client->sendMessage(&req);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceFramebuffer* framebufferResource =
            dynamic_cast<dglnet::resource::DGLResourceFramebuffer*>(
                    reply->m_Reply.get());
    ASSERT_TRUE(framebufferResource != NULL);

    utils::checkColorRect(
            (GLubyte*)framebufferResource->m_PixelRectangle->getPtr(),
            framebufferResource->m_PixelRectangle->m_Width,
            framebufferResource->m_PixelRectangle->m_Height,
            framebufferResource->m_PixelRectangle->m_RowBytes, 204, 127, 102,
            255);

    // Reset shader to default
    {
        dglnet::message::Request req(new dglnet::request::EditShaderSource(
                breaked->m_CurrentCtx, fragId, true));
        client->sendMessage(&req);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));

    // Verify frag shader
    {
        dglnet::message::Request req(new dglnet::request::QueryResource(
                dglnet::message::ObjectType::Shader,
                dglnet::ContextObjectName(breaked->m_CurrentCtx, fragId)));
        client->sendMessage(&req);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    shaderResource = dynamic_cast<dglnet::resource::DGLResourceShader*>(
            reply->m_Reply.get());
    ASSERT_EQ(oldSource, shaderResource->m_Source);
    ASSERT_EQ(shaderResource->m_CompileStatus.second, GL_TRUE);

    terminate(client);
}

TEST_F(LiveTest, shader_handling) {
    dglnet::message::RequestReply* reply;
    std::string nothing;
    GLuint shaderId, programId;

    std::shared_ptr<dglnet::Client> client = getClientFor("shader_handling");

    dglnet::message::BreakedCall* breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
                    client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable other breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    //#simple case: create-delete
    // shader = glCreateShader(GL_VERTEX_SHADER);
    //#test point: we have one shader
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDeleteShader_Call);
    EXPECT_EQ(1, breaked->m_CtxReports[0].m_ShaderSpace.size());
    EXPECT_EQ(GL_VERTEX_SHADER,
              breaked->m_CtxReports[0].m_ShaderSpace.begin()->m_Target);
    // glDeleteShader(shader);
    //#test point: shader deleted
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glFlush_Call);
    EXPECT_EQ(0, breaked->m_CtxReports[0].m_ShaderSpace.size());
    // glFlush(); #flush is only to mark case end

    // shader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(shader, source);
    // glDeleteShader(shader);
    //#test point: shader deleted
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glFlush_Call);
    EXPECT_EQ(0, breaked->m_CtxReports[0].m_ShaderSpace.size());
    // glFlush(); #flush is only to mark case end

    //#lazy deletion test, create-attach-delete-detach
    // shader = glCreateShader(GL_VERTEX_SHADER);
    // program = glCreateProgram();
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glAttachShader_Call);
    breaked->m_entryp.getArgs()[0].get(programId);
    breaked->m_entryp.getArgs()[1].get(shaderId);
    // glAttachShader(program, shader);
    //#test point: program has shader
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDeleteShader_Call);
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Program,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    EXPECT_EQ(1, dynamic_cast<dglnet::resource::DGLResourceProgram*>(
                         reply->m_Reply.get())->m_AttachedShaders.size());
    EXPECT_EQ(shaderId, dynamic_cast<dglnet::resource::DGLResourceProgram*>(
                                reply->m_Reply.get())
                                ->m_AttachedShaders[0]
                                .first);
    // glDeleteShader(shader);
    //#test point: shader not deleted
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDetachShader_Call);
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Shader,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  shaderId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    // glDetachShader(program, shader);
    //#test point: shader deleted, program has no shader
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDeleteProgram_Call);
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Shader,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  shaderId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_FALSE(reply->isOk(nothing));
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Program,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    EXPECT_EQ(0, dynamic_cast<dglnet::resource::DGLResourceProgram*>(
                         reply->m_Reply.get())->m_AttachedShaders.size());
    // glDeleteProgram(program)
    // glFlush(); #flush is only to mark case end

    //#lazy deletion test2: create-attach-delete-deleteprogram
    // shader = glCreateShader(GL_VERTEX_SHADER);
    // program = glCreateProgram();
    // glAttachShader(program, shader);
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glAttachShader_Call);
    breaked->m_entryp.getArgs()[0].get(programId);
    breaked->m_entryp.getArgs()[1].get(shaderId);
    // glDeleteShader(shader);
    //#test point: shader not deleted, program has shader
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glDeleteProgram_Call);
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Shader,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  shaderId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Program,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_TRUE(reply->isOk(nothing));
    EXPECT_EQ(1, dynamic_cast<dglnet::resource::DGLResourceProgram*>(
                         reply->m_Reply.get())->m_AttachedShaders.size());
    EXPECT_EQ(shaderId, dynamic_cast<dglnet::resource::DGLResourceProgram*>(
                                reply->m_Reply.get())
                                ->m_AttachedShaders[0]
                                .first);
    // glDeleteProgram(program)
    //#test point: shader deleted
    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
                                        glFlush_Call);
    {
        dglnet::message::Request requestMessage(
                new dglnet::request::QueryResource(
                        dglnet::message::ObjectType::Shader,
                        dglnet::ContextObjectName(breaked->m_CurrentCtx,
                                                  shaderId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());
    ASSERT_FALSE(reply->isOk(nothing));
    // glFlush(); #flush is only to mark case end
   terminate(client);
}

TEST_F(LiveTest, program_handling) {
    dglnet::message::RequestReply* reply;
    std::string nothing;

    std::shared_ptr<dglnet::Client> client = getClientFor("program_handling");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable other breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glLinkProgram_Call);


    ASSERT_EQ(2, breaked->m_CtxReports[0].m_ShaderSpace.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_ProgramSpace.size());

    GLuint programId = breaked->m_CtxReports[0].m_ProgramSpace.begin()->m_Name;

    {
        dglnet::message::Request requestMessage(
            new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Program,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());

    dglnet::resource::DGLResourceProgram* programResource =
        dynamic_cast<dglnet::resource::DGLResourceProgram*>(
        reply->m_Reply.get());
    ASSERT_TRUE(programResource != NULL);

    EXPECT_FALSE(programResource->mLinkStatus.second);
    EXPECT_EQ(0, programResource->m_Uniforms.size());
    ASSERT_EQ(2, programResource->m_AttachedShaders.size());
    
    if (programResource->m_AttachedShaders[0].second == GL_VERTEX_SHADER) {
        EXPECT_TRUE(programResource->m_AttachedShaders[1].second == GL_FRAGMENT_SHADER);
    } else if (programResource->m_AttachedShaders[0].second == GL_FRAGMENT_SHADER) {
        EXPECT_TRUE(programResource->m_AttachedShaders[1].second == GL_VERTEX_SHADER);
    } else {
        EXPECT_TRUE(0);
    }
    EXPECT_TRUE(programResource->m_AttachedShaders[0].first != programResource->m_AttachedShaders[1].first);
    dglnet::ContextObjectName nameShader1(breaked->m_CtxReports[0].m_Id, programResource->m_AttachedShaders[0].first);
    dglnet::ContextObjectName nameShader2(breaked->m_CtxReports[0].m_Id, programResource->m_AttachedShaders[1].first);

    EXPECT_TRUE(breaked->m_CtxReports[0].m_ShaderSpace.find(nameShader1) !=  breaked->m_CtxReports[0].m_ShaderSpace.end());
    EXPECT_TRUE(breaked->m_CtxReports[0].m_ShaderSpace.find(nameShader2) !=  breaked->m_CtxReports[0].m_ShaderSpace.end());

    // advance one call
    dglnet::message::ContinueBreak stepCall(
        dglnet::message::StepMode::CALL);
    client->sendMessage(&stepCall);
    
    breaked = utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());

    ASSERT_TRUE(breaked != NULL);

    ASSERT_EQ(2, breaked->m_CtxReports[0].m_ShaderSpace.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_ProgramSpace.size());

    //check if linked

    {
        dglnet::message::Request requestMessage(
            new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Program,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());

    programResource =
        dynamic_cast<dglnet::resource::DGLResourceProgram*>(
        reply->m_Reply.get());
    ASSERT_TRUE(programResource != NULL);

    EXPECT_TRUE(programResource->mLinkStatus.second);
    EXPECT_EQ(0, programResource->m_Uniforms.size());
    EXPECT_EQ(2, programResource->m_AttachedShaders.size());

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glDeleteProgram_Call);

    ASSERT_EQ(2, breaked->m_CtxReports[0].m_ShaderSpace.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_ProgramSpace.size());

    //check if linked and still present
    {
        dglnet::message::Request requestMessage(
            new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Program,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());

    programResource =
        dynamic_cast<dglnet::resource::DGLResourceProgram*>(
        reply->m_Reply.get());
    ASSERT_TRUE(programResource != NULL);

    EXPECT_TRUE(programResource->mLinkStatus.second);
    EXPECT_EQ(0, programResource->m_Uniforms.size());
    EXPECT_EQ(2, programResource->m_AttachedShaders.size());

    breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
        glUseProgram_Call);
    
    {
        // advance one call
        dglnet::message::ContinueBreak stepCall2(
            dglnet::message::StepMode::CALL);
        client->sendMessage(&stepCall2);
        breaked =
            utils::receiveUntilMessage<dglnet::message::BreakedCall>(
            client.get(), getMessageHandler());
    }

    EXPECT_EQ(0, breaked->m_CtxReports[0].m_ShaderSpace.size());
    EXPECT_EQ(0, breaked->m_CtxReports[0].m_ProgramSpace.size());

    {
        dglnet::message::Request requestMessage(
            new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Program,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            programId)));
        client->sendMessage(&requestMessage);
    }
    reply = utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    
    std::string tmp;

    EXPECT_FALSE(reply->isOk(tmp));
    terminate(client);
}


TEST_F(LiveTest, fbo_msaa) {
    std::shared_ptr<dglnet::Client> client = getClientFor("fbo_msaa");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    // run through 3-th clear
    for (int i = 0; i < 4; i++) {
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
            glClear_Call);

        // advance one call
        dglnet::message::ContinueBreak stepCall(
            dglnet::message::StepMode::CALL);

        client->sendMessage(&stepCall);
    }

    // query fbo list
    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_FBOSpace.size());

    EXPECT_EQ(0,
        breaked->m_CtxReports[0].m_FBOSpace.begin()->m_Target);

    {
        // query fbo
        dglnet::message::Request request(new dglnet::request::QueryResource(
            dglnet::message::ObjectType::FBO,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            breaked->m_CtxReports[0]
        .m_FBOSpace.begin()
            ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
        utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    std::string nothing;
    EXPECT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceFBO* fboResource =
        dynamic_cast<dglnet::resource::DGLResourceFBO*>(
        reply->m_Reply.get());
    ASSERT_TRUE(fboResource != NULL);

    ASSERT_EQ(3, fboResource->m_Attachments.size());


    //glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 256, 256);
    std::string errorStr;
    EXPECT_EQ(true, fboResource->m_Attachments[0].isOk(errorStr));
    EXPECT_EQ(0, errorStr.size());
    
    ASSERT_EQ(GL_RGBA, fboResource->m_Attachments[0].m_PixelRectangle->m_GLFormat);
    ASSERT_EQ(GL_UNSIGNED_BYTE,
        fboResource->m_Attachments[0].m_PixelRectangle->m_GLType);

    EXPECT_EQ(256, fboResource->m_Attachments[0].m_PixelRectangle->m_Width);
    EXPECT_EQ(256, fboResource->m_Attachments[0].m_PixelRectangle->m_Height);

    utils::checkColor(
        (GLubyte*)fboResource->m_Attachments[0].m_PixelRectangle->getPtr(),
        fboResource->m_Attachments[0].m_PixelRectangle->m_Width,
        fboResource->m_Attachments[0].m_PixelRectangle->m_Height,
        fboResource->m_Attachments[0].m_PixelRectangle->m_RowBytes, 102, 127,
        204, 255);

    EXPECT_EQ(GL_COLOR_ATTACHMENT0, fboResource->m_Attachments[0].m_Id);

    EXPECT_EQ(GL_RGBA8, fboResource->m_Attachments[0].m_Internalformat);
    EXPECT_EQ(4, fboResource->m_Attachments[0].m_Samples);

    //glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 256, 256, 0);

    EXPECT_EQ(true, fboResource->m_Attachments[1].isOk(errorStr));
    EXPECT_EQ(0, errorStr.size());

    ASSERT_EQ(GL_RGBA, fboResource->m_Attachments[1].m_PixelRectangle->m_GLFormat);
    ASSERT_EQ(GL_UNSIGNED_BYTE,
        fboResource->m_Attachments[1].m_PixelRectangle->m_GLType);

    EXPECT_EQ(256, fboResource->m_Attachments[1].m_PixelRectangle->m_Width);
    EXPECT_EQ(256, fboResource->m_Attachments[1].m_PixelRectangle->m_Height);

    utils::checkColor(
        (GLubyte*)fboResource->m_Attachments[1].m_PixelRectangle->getPtr(),
        fboResource->m_Attachments[1].m_PixelRectangle->m_Width,
        fboResource->m_Attachments[1].m_PixelRectangle->m_Height,
        fboResource->m_Attachments[1].m_PixelRectangle->m_RowBytes, 204, 127,
        102, 255);

    EXPECT_EQ(GL_COLOR_ATTACHMENT1, fboResource->m_Attachments[1].m_Id);

    EXPECT_EQ(GL_RGBA8, fboResource->m_Attachments[1].m_Internalformat);
    EXPECT_EQ(4, fboResource->m_Attachments[1].m_Samples);

    //glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4, GL_DEPTH24_STENCIL8, 256, 256, 256, 0);

    EXPECT_EQ(true, fboResource->m_Attachments[2].isOk(errorStr));
    EXPECT_EQ(0, errorStr.size());

    ASSERT_EQ(GL_DEPTH_STENCIL, fboResource->m_Attachments[2].m_PixelRectangle->m_GLFormat);
    ASSERT_EQ(GL_UNSIGNED_INT_24_8,
        fboResource->m_Attachments[2].m_PixelRectangle->m_GLType);

    EXPECT_EQ(256, fboResource->m_Attachments[2].m_PixelRectangle->m_Width);
    EXPECT_EQ(256, fboResource->m_Attachments[2].m_PixelRectangle->m_Height);


    for (int i = 0;
        i < fboResource->m_Attachments[2].m_PixelRectangle->m_Width * fboResource->m_Attachments[2].m_PixelRectangle->m_Height;
        i++) {

         EXPECT_EQ(1717986816, ((GLuint*)fboResource->m_Attachments[2].m_PixelRectangle->getPtr())[i]);
    }
 
    EXPECT_EQ(GL_DEPTH_STENCIL_ATTACHMENT, fboResource->m_Attachments[2].m_Id);

    EXPECT_EQ(GL_DEPTH24_STENCIL8, fboResource->m_Attachments[2].m_Internalformat);
    EXPECT_EQ(4, fboResource->m_Attachments[2].m_Samples);

    terminate(client);
}

TEST_F(LiveTest, renderbuffer_msaa) {
    std::shared_ptr<dglnet::Client> client = getClientFor("fbo_msaa");

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);

    {
        // disable breaking stuff
        dglnet::message::Configuration config(getUsualConfig());
        client->sendMessage(&config);
    }

    // run through 3-th clear
    for (int i = 0; i < 4; i++) {
        breaked = utils::runUntilEntryPoint(client, getMessageHandler(),
            glClear_Call);

        // advance one call
        dglnet::message::ContinueBreak stepCall(
            dglnet::message::StepMode::CALL);
        client->sendMessage(&stepCall);
    }

    // query rbo list
    ASSERT_EQ(1, breaked->m_CtxReports.size());
    ASSERT_EQ(1, breaked->m_CtxReports[0].m_RenderbufferSpace.size());

    EXPECT_EQ(0,
        breaked->m_CtxReports[0].m_RenderbufferSpace.begin()->m_Target);

    {
        // query rbo
        dglnet::message::Request request(new dglnet::request::QueryResource(
            dglnet::message::ObjectType::Renderbuffer,
            dglnet::ContextObjectName(breaked->m_CurrentCtx,
            breaked->m_CtxReports[0]
        .m_RenderbufferSpace.begin()
            ->m_Name)));
        client->sendMessage(&request);
    }

    dglnet::message::RequestReply* reply =
        utils::receiveUntilMessage<dglnet::message::RequestReply>(
        client.get(), getMessageHandler());
    std::string nothing;
    EXPECT_TRUE(reply->isOk(nothing));
    dglnet::resource::DGLResourceRenderbuffer* rboResource =
        dynamic_cast<dglnet::resource::DGLResourceRenderbuffer*>(
        reply->m_Reply.get());
    ASSERT_TRUE(rboResource != NULL);


    //glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 256, 256);
    
    ASSERT_EQ(GL_RGBA, rboResource->m_PixelRectangle->m_GLFormat);
    ASSERT_EQ(GL_UNSIGNED_BYTE,
        rboResource->m_PixelRectangle->m_GLType);

    EXPECT_EQ(256, rboResource->m_PixelRectangle->m_Width);
    EXPECT_EQ(256, rboResource->m_PixelRectangle->m_Height);

    utils::checkColor(
        (GLubyte*)rboResource->m_PixelRectangle->getPtr(),
        rboResource->m_PixelRectangle->m_Width,
        rboResource->m_PixelRectangle->m_Height,
        rboResource->m_PixelRectangle->m_RowBytes, 102, 127,
        204, 255);

    EXPECT_EQ(GL_RGBA8, rboResource->m_Internalformat);
    EXPECT_EQ(4, rboResource->m_Samples);

    terminate(client);
}

#ifdef _WIN32

TEST_F(LiveTest, benchmark) {

#if 1
    std::shared_ptr<dglnet::Client> client = getClientFor("simple");
#else
    std::shared_ptr<dglnet::Client> client =
        dglnet::Client::Create(&m_Controller, &getMessageHandler());

    client->connectServer("127.0.0.1", "8888");
#endif

    dglnet::message::BreakedCall* breaked =
        utils::receiveUntilMessage<dglnet::message::BreakedCall>(
        client.get(), getMessageHandler());
    ASSERT_TRUE(breaked != NULL);


#ifdef _WIN32
    double frequency;
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        frequency = double(f.QuadPart) / 1000000.0;
    }

#endif

    value_t sizes[] = {
        1, 
        10, 
        100, 
        1000, 
        10000, 
        100000, 
        1000000, 
        10000000,
        100000000,
    };

    for (size_t i =0; i < DGL_ARRAY_LENGTH(sizes); i++) {

        LARGE_INTEGER start, stop;

        QueryPerformanceCounter(&start);

        dglnet::message::Request request(new dglnet::request::RequestBenchmarkBuffer(sizes[i]));
        client->sendMessage(&request);

        dglnet::message::RequestReply* reply =
            utils::receiveUntilMessage<dglnet::message::RequestReply>(
            client.get(), getMessageHandler());

        std::string nothing;
        EXPECT_TRUE(reply->isOk(nothing));
        dglnet::DGLBenchmarkBuffer* bBuffer =
            dynamic_cast<dglnet::DGLBenchmarkBuffer*>(reply->m_Reply.get());

        ASSERT_TRUE(bBuffer != NULL);
        ASSERT_EQ(bBuffer->m_Size, sizes[i]);

        QueryPerformanceCounter(&stop);


        std::stringstream propertyNameStr;
        propertyNameStr << "PerSize" << sizes[i];
        std::string perfName = propertyNameStr.str();

        RecordProperty(perfName.c_str(), (int)(double(stop.QuadPart - start.QuadPart) / frequency));
    }    

    terminate(client);
}

#endif //_WIN32

}    // namespace
