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
    DGLSyntaxHighlighterGLSL highlighter(false, editor.document());
}

template <typename T>
class vector_inserter {
   public:
    std::deque<T> v;
    vector_inserter& operator()(T const& val) {
        v.push_back(val);
        return *this;
    }
    vector_inserter& operator+(std::vector<T> vectVal) {
        std::copy(vectVal.begin(), vectVal.end(), std::back_inserter(v));
        return *this;
    }
    vector_inserter& operator, (T val) { return (*this)(val); }

    template <typename T2>
    operator std::vector<T2>() const {
        return std::vector<T2>(v.begin(), v.end());
    }
};
template <typename T>
vector_inserter<T> list_of(const T& val) {
    return vector_inserter<T>()(val);
}

namespace {
enum class Format {
    UNFORMATTED,
    KEYWORD,
    KEYWORDBOLD,
    KEYWORD_RESERVED,
    BUILD_IN,
    NUMBER,
};

#define DECL_FORMAT(X)                \
    struct X {                        \
        X(const char* s) : m_s(s) {}; \
        std::string m_s;              \
    }

DECL_FORMAT(U);
DECL_FORMAT(K);
DECL_FORMAT(B);
DECL_FORMAT(R);
DECL_FORMAT(I);
DECL_FORMAT(N);

struct S {
    S() {};
    S(U u) : m_s(u.m_s), m_format(Format::UNFORMATTED) {};
    S(N n) : m_s(n.m_s), m_format(Format::NUMBER) {};
    S(K k) : m_s(k.m_s), m_format(Format::KEYWORD) {};
    S(B b) : m_s(b.m_s), m_format(Format::KEYWORDBOLD) {};
    S(R r) : m_s(r.m_s), m_format(Format::KEYWORD_RESERVED) {};
    S(I i) : m_s(i.m_s), m_format(Format::BUILD_IN) {};
    std::string m_s;
    Format m_format;
};

struct ExpFormat {
    int start;
    int length;
    Format format;
    ExpFormat(int s, int l, Format f) : start(s), length(l), format(f) {}
};

std::vector<std::vector<ExpFormat> > getExpectedHL(
        const std::vector<std::vector<S> >& shaderStrings,
        std::ostringstream& shaderStream) {
    std::vector<std::vector<ExpFormat> > expected;
    for (size_t i = 0; i < shaderStrings.size(); i++) {
        std::vector<ExpFormat> e;
        size_t l = 0;
        for (size_t j = 0; j < shaderStrings[i].size(); j++) {
            shaderStream << shaderStrings[i][j].m_s;
            if (shaderStrings[i][j].m_s.size()) {
                e.push_back(ExpFormat(l, shaderStrings[i][j].m_s.size(),
                                      shaderStrings[i][j].m_format));
            }
            l += shaderStrings[i][j].m_s.size();
        }
        shaderStream << std::endl;
        expected.push_back(e);
    }
    expected.push_back(std::vector<ExpFormat>());
    return expected;
}

void validateHL(QPlainTextEdit* editor,
                const std::vector<std::vector<ExpFormat> >& expected,
                std::vector<std::vector<S> >& /*strings*/) {
    int line = 0;
    for (QTextBlock it = editor->document()->begin();
         it != editor->document()->end(); it = it.next()) {
        QTextLayout* layout = it.layout();

        // cout << "{ ";
        int format = 0;

        QList<QTextLayout::FormatRange> formatRangeList =
                layout->additionalFormats();

        for (QList<QTextLayout::FormatRange>::iterator i =
                     formatRangeList.begin();
             i != formatRangeList.end(); ++i) {
            // cout << "{ " << i->start << ", " << i->length << " }, ";

            ASSERT_TRUE(format < (int)expected[line].size());

            // cout << "|" << strings[line][format].m_s << "|" << endl;

            EXPECT_EQ(expected[line][format].start, i->start);
            EXPECT_EQ(expected[line][format].length, i->length);

            bool expectedStrikeout = false;
            QColor expectedColor;
            int expectedFontWeight = QFont::Normal;
            switch (expected[line][format].format) {
                case Format::NUMBER:
                    expectedColor = Qt::darkMagenta;
                    expectedFontWeight = QFont::Bold;
                    break;
                case Format::KEYWORDBOLD:
                    expectedFontWeight = QFont::Bold;
                case Format::KEYWORD:
                    expectedColor = Qt::darkYellow;
                    break;
                case Format::KEYWORD_RESERVED:
                    expectedFontWeight = QFont::Bold;
                    expectedStrikeout = true;
                    expectedColor = Qt::darkYellow;
                    break;
                case Format::UNFORMATTED:
                    expectedColor = Qt::black;
                    break;
                case Format::BUILD_IN:
                    expectedColor = Qt::darkMagenta;
                    break;
            }

            // cout << i->format.foreground().color().red() << ", " <<
            // i->format.foreground().color().green() << ", " <<
            // i->format.foreground().color().blue() << ", " <<
            // i->format.foreground().color().alpha() << endl;
            // cout << expectedColor.red() << ", " << expectedColor.green() <<
            // ", " << expectedColor.blue() << ", " << expectedColor.alpha() <<
            // endl;

            EXPECT_EQ(expectedColor, i->format.foreground().color());

            EXPECT_EQ(expectedFontWeight, i->format.fontWeight());

            EXPECT_EQ(expectedStrikeout, i->format.fontStrikeOut());

            format++;
        }

        // cout << "},\n";

        ASSERT_TRUE(line < (int)expected.size());
        line++;
    }
}
};

TEST_F(DGLGui, syntax_highlighter_parse) {
    QPlainTextEdit editor;
    DGLSyntaxHighlighterGLSL highlighter(false, editor.document());

    std::vector<std::vector<S> > strings =
            (list_of(list_of((S)(N) "#version 330")), list_of((S)(U) ""),
             list_of((S)(B) "uniform")((U) " ")((K) "sampler2D")(
                     (U) " samplerImg;"),
             list_of((S)(B) "in")((U) " ")((K) "vec2")((U) " texcoord;"),
             list_of((S)(K) "void")((U) " main() {"),
             list_of((S)(U) "    ")((K) "vec4")((U) " texcolor = ")(
                     (B) "texture2D")((U) "(samplerImg,texcoord);"),
             list_of((S)(U) "    ")((B) "gl_FragColor")((U) " = texcolor;"),
             list_of((S)(U) "}"));

    std::ostringstream shader;
    std::vector<std::vector<ExpFormat> > expected =
            getExpectedHL(strings, shader);

    editor.appendPlainText(shader.str().c_str());

    // cout << shader.str();

    QCoreApplication::processEvents();

    EXPECT_NO_FATAL_FAILURE(validateHL(&editor, expected, strings));
}

TEST_F(DGLGui, syntax_highlighter_parse_esslflag) {
    bool essl[2] = {false, true};

    for (int i = 0; i < 2; i++) {
        QPlainTextEdit editor;

        std::vector<S> dFdx;
        if (!essl[i]) {
            dFdx = list_of((S)(B) "dFdx");
        } else {
            dFdx = list_of((S)(R) "dFdx");
        }

        DGLSyntaxHighlighterGLSL highlighter(essl[i], editor.document());
        std::vector<std::vector<S> > strings =
                (list_of(list_of((S)(B) "in")((U) " ")((K) "vec4")(
                         (U) " color;")),
                 list_of((S)(K) "void")((U) " main() {"),
                 (list_of((S)(U) "    ")((I) "gl_FragColor")((U) " = ") +
                  dFdx)((U) "(color);"),
                 list_of((S)(U) "}"));

        std::ostringstream shader;
        std::vector<std::vector<ExpFormat> > expected =
                getExpectedHL(strings, shader);

        editor.appendPlainText(shader.str().c_str());

        cout << shader.str();

        QCoreApplication::processEvents();

        EXPECT_NO_FATAL_FAILURE(validateHL(&editor, expected, strings));
    }
}
}
