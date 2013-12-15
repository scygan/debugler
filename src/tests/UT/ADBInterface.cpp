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

#include <DGLGUI/dgladbinterface.h>



namespace {

namespace mock {

    class DGLAdbCookieMock: public DGLAdbCookie {
    public:
        DGLAdbCookieMock(std::shared_ptr<DGLAdbOutputFilter> filter):DGLAdbCookie(filter) {}
    private:
        virtual void process() override {

        }
    };

    class DGLAdbCookieFactoryMock: public DGLAdbCookieFactoryBase {
    private:
        virtual DGLAdbCookie* CreateCookie(const std::vector<std::string>& params,
            std::shared_ptr<DGLAdbOutputFilter> filter) override {
                return new DGLAdbCookieMock(/*m_adbPath, params*/ filter);
        }
    };
}


// The fixture for testing class Foo.
class AdbInterface : public ::testing::Test {
   protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    AdbInterface() {
        // You can do set-up work for each test here.
    }

    virtual ~AdbInterface() {
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
TEST_F(AdbInterface, empty) {
    
}


}    // namespace
