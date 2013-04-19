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

#include <DGLCommon/gl-types.h>
#include <DGLCommon/gl-formats.h>
#include <DGLCommon/os.h>

#include <DGLWrapper/api-loader.h>

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
        ASSERT_STREQ(GetEntryPointName(glEnable_Call), "glEnable");
        
        //glext.h
        ASSERT_STREQ(GetEntryPointName(glDrawArraysInstancedBaseInstance_Call), "glDrawArraysInstancedBaseInstance");
        ASSERT_STREQ(GetEntryPointName(glDrawArraysInstancedARB_Call), "glDrawArraysInstancedARB");

        //wgl
        ASSERT_STREQ(GetEntryPointName(wglCreateContext_Call), "wglCreateContext");

        //wgl-notrace
        ASSERT_STREQ(GetEntryPointName(wglSetPixelFormat_Call), "wglSetPixelFormat");
        
        //wglext.h
        ASSERT_STREQ(GetEntryPointName(wglCreateContextAttribsARB_Call), "wglCreateContextAttribsARB");

        //egl.h
        ASSERT_STREQ(GetEntryPointName(eglBindAPI_Call), "eglBindAPI");

        ASSERT_STREQ(GetEntryPointName(eglCreateContext_Call), "eglCreateContext");
        ASSERT_STREQ(GetEntryPointName(eglMakeCurrent_Call), "eglMakeCurrent");
        ASSERT_STREQ(GetEntryPointName(eglGetProcAddress_Call), "eglGetProcAddress");

        //eglext.h
        ASSERT_STREQ(GetEntryPointName(eglCreateImageKHR_Call), "eglCreateImageKHR");
        ASSERT_STREQ(GetEntryPointName(eglQuerySurfacePointerANGLE_Call), "eglQuerySurfacePointerANGLE");

        //glx.h
        ASSERT_STREQ(GetEntryPointName(glXChooseFBConfig_Call), "glXChooseFBConfig");

        const char* null = NULL;
        //null
        ASSERT_STREQ(GetEntryPointName(NO_ENTRYPOINT), "<unknown>");
    }

    //here direct pointers are kept (pointers to entrypoints exposed by underlying OpenGL32 implementation
    //use DIRECT_CALL(name) to call one of these pointers
    int ut_PointerLibraries[Entrypoints_NUM] = {
#define FUNC_LIST_ELEM_SUPPORTED(name, type, library)  library,
#define FUNC_LIST_ELEM_NOT_SUPPORTED(name, type, library) FUNC_LIST_ELEM_SUPPORTED(name, type, library)
#include "codegen/functionList.inl"
#undef FUNC_LIST_ELEM_SUPPORTED
#undef FUNC_LIST_ELEM_NOT_SUPPORTED
    };

    TEST_F(DGLCommonUT, codegen_libraries) {
        //gl.h + gl2.h
        EXPECT_EQ(LIBRARY_GL | LIBRARY_ES2 | LIBRARY_ES3, ut_PointerLibraries[glEnable_Call]);

        //all ES2 entryps are should be shared with GL or GL_EXT
        for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
            if (ut_PointerLibraries[i] & LIBRARY_ES2) {
                EXPECT_GT(ut_PointerLibraries[i] & (LIBRARY_GL | LIBRARY_GL_EXT), 0);
            }
        }

        //glext.h
        EXPECT_EQ(LIBRARY_GL_EXT, ut_PointerLibraries[glDrawArraysInstancedBaseInstance_Call]);
        EXPECT_EQ(LIBRARY_GL_EXT, ut_PointerLibraries[glDrawArraysInstancedARB_Call]);

        //wgl
        EXPECT_EQ(LIBRARY_WGL, ut_PointerLibraries[wglCreateContext_Call]);

        //wgl-notrace
        EXPECT_EQ(LIBRARY_WGL, ut_PointerLibraries[wglSetPixelFormat_Call]);

        //wglext.h
        EXPECT_EQ(LIBRARY_WGL_EXT, ut_PointerLibraries[wglCreateContextAttribsARB_Call]);

        //egl.h
        EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglBindAPI_Call]);

        EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglCreateContext_Call]);
        EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglMakeCurrent_Call]);
        EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglGetProcAddress_Call]);

        //eglext.h
        EXPECT_EQ(LIBRARY_EGL_EXT, ut_PointerLibraries[eglCreateImageKHR_Call]);

        //glx.h
        EXPECT_EQ(LIBRARY_GLX, ut_PointerLibraries[glXChooseFBConfig_Call]);

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
        EXPECT_EQ(rgba8.getType(), GL_UNSIGNED_BYTE);
    }


    TEST_F(DGLCommonUT, os_env) {
        EXPECT_EQ("", Os::getEnv("test_name"));
        Os::setEnv("test_name", "test_value");
        EXPECT_EQ("test_value", Os::getEnv("test_name"));
    }


}  // namespace

