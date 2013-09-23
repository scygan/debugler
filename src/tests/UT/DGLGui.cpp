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
    
    template <typename T> class vector_inserter{
    public:
        std::deque<T> v;
        vector_inserter& operator()(T const& val) { v.push_back(val);return *this; }
        vector_inserter& operator,(T val){ return (*this)(val); }

        template <typename T2>
        operator std::vector<T2>() const { return std::vector<T2>(v.begin(), v.end()); }
    };
    template<typename T>
    vector_inserter<T> list_of(const T& val) {
        return vector_inserter<T>()(val);
    }

    TEST_F(DGLGui, syntax_highlighter_parse) {
        QPlainTextEdit editor;
        DGLSyntaxHighlighterGLSL highlighter(editor.document());

        std::vector<std::vector<std::string> > strings = (
            list_of(list_of((std::string)"#version 330")),
                    list_of((std::string)""),
                    list_of((std::string)"uniform")(" ")("sampler2D")(" img;"),
                    list_of((std::string)"in")(" ")("vec2")(" texcoord;"),
                    list_of((std::string)"void")(" main() {"),
                    list_of((std::string)"    ")("vec4")(" texcolor = ")("texture2D")("(img,texcoord);"),
                    list_of((std::string)"    ")("gl_FragColor")(" = texcolor;"),
                    list_of((std::string)"}"));     
  
        

        struct ExpFormat {
            int start;
            int length;
            ExpFormat(int s, int l):start(s),length(l) {}
        };
        std::vector<std::vector<ExpFormat> > expected;

        std::ostringstream shader;
        for (size_t i = 0; i < strings.size(); i++) {
            std::vector<ExpFormat> e;
            size_t l = 0;
            for (size_t j = 0; j < strings[i].size(); j++) {
                shader << strings[i][j];
                if (strings[i][j].size()) {
                    e.push_back(ExpFormat(l, strings[i][j].size()));
                }
                l += strings[i][j].size();
            }
            shader << std::endl;
            expected.push_back(e);
        }

        expected.push_back(std::vector<ExpFormat>());
        
        editor.appendPlainText(shader.str().c_str());

        //cout << shader.str();

        QCoreApplication::processEvents();

        int line = 0;
        for (QTextBlock it = editor.document()->begin(); it != editor.document()->end(); it = it.next()) {
            QTextLayout * layout = it.layout();

            //cout << "{ ";
            int format = 0;
            
            QList<QTextLayout::FormatRange>  formatRangeList = layout->additionalFormats();

            for (QList<QTextLayout::FormatRange>::iterator i = formatRangeList.begin(); i != formatRangeList.end(); ++i) {
              //  cout << "{ " << i->start << ", " << i->length << " }, ";

                ASSERT_TRUE(format < (int) expected[line].size());

                EXPECT_EQ(expected[line][format].start, i->start);
                EXPECT_EQ(expected[line][format].length, i->length);

                format++;
            }

            //cout << "},\n";

            ASSERT_TRUE(line < (int)expected.size());
            line++;
        }


    }
}
