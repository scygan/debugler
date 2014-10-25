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
    // gl.h + gl2.h
    ASSERT_STREQ(GetEntryPointName(glEnable_Call), "glEnable");

    // glext.h
    ASSERT_STREQ(GetEntryPointName(glDrawArraysInstancedBaseInstance_Call),
                 "glDrawArraysInstancedBaseInstance");
    ASSERT_STREQ(GetEntryPointName(glDrawArraysInstancedARB_Call),
                 "glDrawArraysInstancedARB");

    // wgl
    ASSERT_STREQ(GetEntryPointName(wglCreateContext_Call), "wglCreateContext");

    // wgl-notrace
    ASSERT_STREQ(GetEntryPointName(wglSetPixelFormat_Call),
                 "wglSetPixelFormat");

    // wglext.h
    ASSERT_STREQ(GetEntryPointName(wglCreateContextAttribsARB_Call),
                 "wglCreateContextAttribsARB");

    // egl.h
    ASSERT_STREQ(GetEntryPointName(eglBindAPI_Call), "eglBindAPI");

    ASSERT_STREQ(GetEntryPointName(eglCreateContext_Call), "eglCreateContext");
    ASSERT_STREQ(GetEntryPointName(eglMakeCurrent_Call), "eglMakeCurrent");
    ASSERT_STREQ(GetEntryPointName(eglGetProcAddress_Call),
                 "eglGetProcAddress");

    // eglext.h
    ASSERT_STREQ(GetEntryPointName(eglCreateImageKHR_Call),
                 "eglCreateImageKHR");
    ASSERT_STREQ(GetEntryPointName(eglQuerySurfacePointerANGLE_Call),
                 "eglQuerySurfacePointerANGLE");

    // glx.h
    ASSERT_STREQ(GetEntryPointName(glXChooseFBConfig_Call),
                 "glXChooseFBConfig");
    ASSERT_STREQ(GetEntryPointName(glXGetProcAddress_Call),
                 "glXGetProcAddress");

    // glxext.h
    ASSERT_STREQ(GetEntryPointName(glXCreateContextAttribsARB_Call),
                 "glXCreateContextAttribsARB");

    // null
    ASSERT_STREQ(GetEntryPointName(NO_ENTRYPOINT), "<unknown>");
}


#define HAVE_LIBRARY_GL
#define HAVE_LIBRARY_GL_EXT
#define HAVE_LIBRARY_EGL
#define HAVE_LIBRARY_EGL_EXT
#define HAVE_LIBRARY_ES1
#define HAVE_LIBRARY_ES1_EXT
#define HAVE_LIBRARY_ES2
#define HAVE_LIBRARY_ES2_EXT
#define HAVE_LIBRARY_ES3
#define HAVE_LIBRARY_WGL
#define HAVE_LIBRARY_WGL_EXT
#define HAVE_LIBRARY_WINGDI
#define HAVE_LIBRARY_GLX
#define HAVE_LIBRARY_GLX_EXT
// here direct pointers are kept (pointers to entrypoints exposed by underlying
// OpenGL32 implementation
// use DIRECT_CALL(name) to call one of these pointers
int ut_PointerLibraries[Entrypoints_NUM] = {
#define FUNC_LIST_SUPPORTED_ELEM(name, type, library, retVal, params) library,
#define FUNC_LIST_NOT_SUPPORTED_ELEM(name, type, library, retVal, params) \
    FUNC_LIST_SUPPORTED_ELEM(name, type, library, params)
#include "codegen_gl_function_list.inl"
#undef FUNC_LIST_SUPPORTED_ELEM
#undef FUNC_LIST_NOT_SUPPORTED_ELEM
};

TEST_F(DGLCommonUT, codegen_libraries) {
    // gl.h + gl2.h
    EXPECT_EQ(LIBRARY_GL | LIBRARY_ES2 | LIBRARY_ES1,
              ut_PointerLibraries[glEnable_Call]);

    // all ES2 entryps are should be shared with GL or GL_EXT
    for (int i = 0; i < NUM_ENTRYPOINTS; i++) {
        if (ut_PointerLibraries[i] & LIBRARY_ES2) {
            EXPECT_GT(ut_PointerLibraries[i] & (LIBRARY_GL | LIBRARY_GL_EXT),
                      0);
        }
    }

    EXPECT_EQ(LIBRARY_GL_EXT | LIBRARY_ES3,
        ut_PointerLibraries[glTransformFeedbackVaryings_Call]);

    // glext.h
    EXPECT_EQ(LIBRARY_GL_EXT,
              ut_PointerLibraries[glDrawArraysInstancedBaseInstance_Call]);
    EXPECT_EQ(LIBRARY_GL_EXT,
              ut_PointerLibraries[glDrawArraysInstancedARB_Call]);

    // wgl
    EXPECT_EQ(LIBRARY_WGL, ut_PointerLibraries[wglCreateContext_Call]);

    // wgl-notrace
    EXPECT_EQ(LIBRARY_WGL, ut_PointerLibraries[wglSetPixelFormat_Call]);

    // wglext.h
    EXPECT_EQ(LIBRARY_WGL_EXT,
              ut_PointerLibraries[wglCreateContextAttribsARB_Call]);

    //wingdi 
    EXPECT_EQ(LIBRARY_WINGDI,
        ut_PointerLibraries[SwapBuffers_Call]);
    EXPECT_EQ(LIBRARY_WINGDI,
        ut_PointerLibraries[SetPixelFormat_Call]);
    EXPECT_EQ(LIBRARY_WINGDI,
        ut_PointerLibraries[ChoosePixelFormat_Call]);

    // egl.h
    EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglBindAPI_Call]);

    EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglCreateContext_Call]);
    EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglMakeCurrent_Call]);
    EXPECT_EQ(LIBRARY_EGL, ut_PointerLibraries[eglGetProcAddress_Call]);

    // eglext.h
    EXPECT_EQ(LIBRARY_EGL_EXT, ut_PointerLibraries[eglCreateImageKHR_Call]);

    // glx.h
    EXPECT_EQ(LIBRARY_GLX,
              ut_PointerLibraries[glXChooseFBConfig_Call]);

    // glxext.h
    EXPECT_EQ(LIBRARY_GLX_EXT,
              ut_PointerLibraries[glXCreateContextAttribsARB_Call]);
}

TEST_F(DGLCommonUT, codegen_entryp_names) {
    EXPECT_EQ(GetEntryPointEnum("bad"), NO_ENTRYPOINT);
    EXPECT_EQ(GetEntryPointEnum("glDrawArrays"), glDrawArrays_Call);
    EXPECT_STREQ(GetEntryPointName(GetEntryPointEnum("glDrawArrays")),
                 "glDrawArrays");
    EXPECT_EQ(GetEntryPointEnum(GetEntryPointName(glDrawArrays_Call)),
              glDrawArrays_Call);
}

TEST_F(DGLCommonUT, codegen_entryp_params) {
    {
        GLParamTypeMetadata descr = GetEntryPointGLParamTypeMetadata(glDrawArrays_Call, 0);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Enum);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::PrimitiveType);
    }
    {
        GLParamTypeMetadata descr = GetEntryPointGLParamTypeMetadata(glDrawArrays_Call, 1);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Value);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::NoneGroup);
    }
    {
        GLParamTypeMetadata descr = GetEntryPointGLParamTypeMetadata(glTexImage2D_Call, 2);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Enum);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::TextureComponentCount);
    }
    {
        GLParamTypeMetadata descr = GetEntryPointGLParamTypeMetadata(glClear_Call, 0);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Bitfield);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::ClearBufferMask);
    }
}

TEST_F(DGLCommonUT, codegen_entryp_retval) {
    {
        GLParamTypeMetadata descr = GetEntryPointRetvalMetadata(glDrawArrays_Call);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Value);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::NoneGroup);
    }
    {
        GLParamTypeMetadata descr = GetEntryPointRetvalMetadata(glGetError_Call);
        EXPECT_EQ(descr.m_BaseType, GLParamTypeMetadata::BaseType::Enum);
        EXPECT_EQ(descr.m_EnumGroup, GLEnumGroup::ErrorCode);
    }
}

TEST_F(DGLCommonUT, os_env) {
    EXPECT_EQ("", Os::getEnv("test_name"));
    Os::setEnv("test_name", "test_value");
    EXPECT_EQ("test_value", Os::getEnv("test_name"));
}

}    // namespace
