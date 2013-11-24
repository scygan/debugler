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

#include <DGLNet/protocol/pixeltransfer.h>

namespace {

// The fixture for testing class Foo.
class DGLNetUT : public ::testing::Test {};

TEST_F(DGLNetUT, formats_iformat) {
    DGLPixelTransfer xfer; xfer.initializeOGL(GL_RGBA8, std::vector<GLint>(), std::vector<GLint>());
    EXPECT_EQ(xfer.getFormat(), GL_RGBA);
    EXPECT_EQ(xfer.getType(), GL_UNSIGNED_BYTE);
}

TEST_F(DGLNetUT, formats_noiformat) {

    std::vector<GLint> rgbaSizes(4, 0);
    rgbaSizes[0] = rgbaSizes[1] = rgbaSizes[2] = 8;
    std::vector<GLint> dsSizes(2, 0);

    DGLPixelTransfer xfer; xfer.initializeOGL(0, rgbaSizes, dsSizes);
    EXPECT_EQ(xfer.getFormat(), GL_RGB);
    EXPECT_EQ(xfer.getType(), GL_UNSIGNED_BYTE);
}

TEST_F(DGLNetUT, formats_esformat) {
    {
        DGLPixelTransfer xfer; xfer.initializeOGLES(GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_BYTE);
        EXPECT_EQ(xfer.getFormat(), GL_RGBA);
        EXPECT_EQ(xfer.getType(), GL_UNSIGNED_INT_2_10_10_10_REV);
    } {
        DGLPixelTransfer xfer; xfer.initializeOGLES(GL_RGB4, GL_RGBA, GL_UNSIGNED_SHORT);
        EXPECT_EQ(xfer.getFormat(), GL_RGBA);
        EXPECT_EQ(xfer.getType(), GL_UNSIGNED_SHORT);
    } {
        DGLPixelTransfer xfer; xfer.initializeOGLES(GL_RGB4, 0, 0);
        EXPECT_EQ(xfer.getFormat(), GL_RGBA);
        EXPECT_EQ(xfer.getType(), GL_UNSIGNED_BYTE);
    }
}

}    // namespace
