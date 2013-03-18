
#include "gtest/gtest.h"

#include <DGLCommon/gl-types.h>
#include <DGLCommon/gl-formats.h>
#include <DGLCommon/os.h>

#include <DGLWrapper/api-loader.h>
#include <DGLWrapper/pointers.h>

namespace {

    // The fixture for testing class Foo.
    class DGLCommonUT : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.

        DGLCommonUT() {
            // You can do set-up work for each test here.
        }

        virtual ~DGLCommonUT() {
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

    // Smoke test all inputs of codegen has been parsed to functionList
    TEST_F(DGLCommonUT, codegen_entryps) {
        //gl.h + gl2.h
        EXPECT_EQ(GetEntryPointName(glEnable_Call), "glEnable");
        
        //glext.h
        EXPECT_EQ(GetEntryPointName(glDrawArraysInstancedBaseInstance_Call), "glDrawArraysInstancedBaseInstance");
        EXPECT_EQ(GetEntryPointName(glDrawArraysInstancedARB_Call), "glDrawArraysInstancedARB");

        //wgl
        EXPECT_EQ(GetEntryPointName(wglCreateContext_Call), "wglCreateContext");

        //wgl-notrace
        EXPECT_EQ(GetEntryPointName(wglSetPixelFormat_Call), "wglSetPixelFormat");
        
        //wglext.h
        EXPECT_EQ(GetEntryPointName(wglCreateContextAttribsARB_Call), "wglCreateContextAttribsARB");

        //egl.h
        EXPECT_EQ(GetEntryPointName(eglBindAPI_Call), "eglBindAPI");

        EXPECT_EQ(GetEntryPointName(eglCreateContext_Call), "eglCreateContext");
        EXPECT_EQ(GetEntryPointName(eglMakeCurrent_Call), "eglMakeCurrent");
        EXPECT_EQ(GetEntryPointName(eglGetProcAddress_Call), "eglGetProcAddress_Call");

        //eglext.h
        EXPECT_EQ(GetEntryPointName(eglCreateImageKHR_Call), "eglCreateImageKHR");

        const char* null = NULL;
        //null
        EXPECT_EQ(std::string(GetEntryPointName(NO_ENTRYPOINT)), "<unknown>");
    }

    TEST_F(DGLCommonUT, codegen_libraries) {
        //gl.h + gl2.h
        EXPECT_EQ(LIBRARY_GL | LIBRARY_ES2, g_DirectPointers[glEnable_Call].libraryMask);

        //all ES2 entryps are should be shared with GL or GL_EXT
        for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
            if (g_DirectPointers[i].libraryMask & LIBRARY_ES2) {
                EXPECT_GT(g_DirectPointers[i].libraryMask & (LIBRARY_GL | LIBRARY_GL_EXT), 0);
            }
        }

        //glext.h
        EXPECT_EQ(LIBRARY_GL, g_DirectPointers[glDrawArraysInstancedBaseInstance_Call].libraryMask);
        EXPECT_EQ(LIBRARY_GL, g_DirectPointers[glDrawArraysInstancedARB_Call].libraryMask);

        //wgl
        EXPECT_EQ(LIBRARY_WGL, g_DirectPointers[wglCreateContext_Call].libraryMask);

        //wgl-notrace
        EXPECT_EQ(LIBRARY_WGL, g_DirectPointers[wglSetPixelFormat_Call].libraryMask);

        //wglext.h
        EXPECT_EQ(LIBRARY_WGL_EXT, g_DirectPointers[wglCreateContextAttribsARB_Call].libraryMask);

        //egl.h
        EXPECT_EQ(LIBRARY_EGL, g_DirectPointers[eglBindAPI_Call].libraryMask);

        EXPECT_EQ(LIBRARY_EGL, g_DirectPointers[eglCreateContext_Call].libraryMask);
        EXPECT_EQ(LIBRARY_EGL, g_DirectPointers[eglMakeCurrent_Call].libraryMask);
        EXPECT_EQ(LIBRARY_EGL, g_DirectPointers[eglGetProcAddress_Call].libraryMask);

        //eglext.h
        EXPECT_EQ(LIBRARY_EGL_EXT, g_DirectPointers[eglCreateImageKHR_Call].libraryMask);


        EXPECT_EQ(0, g_DirectPointers[NUM_ENTRYPOINTS].libraryMask);
    }

    TEST_F(DGLCommonUT, codegen_entryp_names) {
        EXPECT_EQ(GetEntryPointEnum("bad"), NO_ENTRYPOINT);
        EXPECT_EQ(GetEntryPointEnum("glDrawArrays"), glDrawArrays_Call);
        EXPECT_EQ(GetEntryPointName(GetEntryPointEnum("glDrawArrays")), "glDrawArrays");
        EXPECT_EQ(GetEntryPointEnum(GetEntryPointName(glDrawArrays_Call)), glDrawArrays_Call);
    }

    TEST_F(DGLCommonUT, formats_iformat) {
       DGLPixelTransfer rgba8(std::vector<GLint>(), std::vector<GLint>(), GL_RGBA8);
       EXPECT_EQ(rgba8.getFormat(), GL_RGBA);
       EXPECT_EQ(rgba8.getType(), GL_UNSIGNED_BYTE);

    }

    TEST_F(DGLCommonUT, formats_noiformat) {

        std::vector<GLint>rgbaSizes(4, 0);
        rgbaSizes[0] = rgbaSizes[1] = rgbaSizes[2] = 8;
        std::vector<GLint>dsSizes(2, 0);

        DGLPixelTransfer rgba8(rgbaSizes, dsSizes, 0);
        EXPECT_EQ(rgba8.getFormat(), GL_RGB);
        EXPECT_EQ(rgba8.getType(), GL_FLOAT);
    }


    TEST_F(DGLCommonUT, os_env) {
        EXPECT_EQ("", Os::getEnv("test_name"));
        Os::setEnv("test_name", "test_value");
        EXPECT_EQ("test_value", Os::getEnv("test_name"));
    }


}  // namespace

