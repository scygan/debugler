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

#include <DGLGui/dglsyntaxhighlight.h>

#include <QPlainTextEdit>

#include <iostream>

using namespace std;

namespace {

    // The fixture for testing class Foo.
    class DGLGui : public ::testing::Test {
    protected:

        DGLGui() {}

        virtual ~DGLGui() {}

        virtual void SetUp() {}

        virtual void TearDown() {}
    };

    TEST_F(DGLGui, syntax_highlighter_create) {
        QPlainTextEdit editor;
        DGLSyntaxHighlighterGLSL highlighter(editor.document());
    }

    TEST_F(DGLGui, syntax_highlighter_parse150) {
        QPlainTextEdit editor;
        DGLSyntaxHighlighterGLSL highlighter(editor.document());

        const char* shader = 
            "#version 330\n"
            "\n"
            "uniform sampler2D img;\n"
            "in vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    vec4 texcolor = texture2D(img,texcoord);\n"
            "    gl_FragColor = texcolor;\n"
            "}\n";

        struct expectedRange  {
            int start;
            int length;
        };

        std::vector<std::vector<expectedRange> > expected = {
            
        };

        editor.appendPlainText(shader);

        QCoreApplication::processEvents();

        int line = 0;
        for (QTextBlock it = editor.document()->begin(); it != editor.document()->end(); it = it.next()) {
            QTextLayout * layout = it.layout();

            cout << "{ ";
            int format = 0;
            for (QList<QTextLayout::FormatRange>::iterator i = layout->additionalFormats().begin(); i != layout->additionalFormats().end(); ++i) {
                  cout << "{ " << i->start << ", " << i->length << " }, ";

//                ASSERT_TRUE(format < (int) expected[line].size());

//                EXPECT_EQ(expected[line][format].start, i->start);
//                EXPECT_EQ(expected[line][format].length, i->length);

                format++;
            }

            cout << "},\n";

//            ASSERT_TRUE(line < (int)expected.size());
            line++;
        }


    }
}
