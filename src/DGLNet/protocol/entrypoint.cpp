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

#include "entrypoint.h"

#include <sstream>
#include <iomanip>
#include <cstring>

CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, int numArgs)
        : m_entryp(entryp), m_glError(GL_NO_ERROR), m_SavedArgsCount(0) {
    m_args.resize(numArgs);
}

Entrypoint CalledEntryPoint::getEntrypoint() const { return m_entryp; }

void CalledEntryPoint::setRetVal(const RetValue& ret) { m_retVal = ret; }

void CalledEntryPoint::setError(gl_t error) { m_glError = error; }

void CalledEntryPoint::setDebugOutput(const std::string& message) {
    m_DebugOutput = message;
}

const std::vector<AnyValue>& CalledEntryPoint::getArgs() const {
    return m_args;
}

class AnyValueWriter : public boost::static_visitor<void> {
   public:
    AnyValueWriter(GLEnumGroup enumGroup, std::ostringstream& stream) : m_Stream(&stream), m_EnumGroup(enumGroup) {}

    void operator()(signed long long i) const { (*m_Stream) << i; }
    void operator()(unsigned long long i) const { (*m_Stream) << i; }
    void operator()(signed long i) const { (*m_Stream) << i; }
    void operator()(unsigned long i) const { (*m_Stream) << i; }
    void operator()(unsigned int i) const { (*m_Stream) << i; }
    void operator()(signed int i) const { (*m_Stream) << i; }
    void operator()(unsigned short i) const { (*m_Stream) << i; }
    void operator()(signed short i) const { (*m_Stream) << i; }
    void operator()(unsigned char i) const { (*m_Stream) << (unsigned int)i; }
    void operator()(signed char i) const { (*m_Stream) << (int)i; }
    void operator()(float f) const { (*m_Stream) << f; }
    void operator()(double d) const { (*m_Stream) << d; }
    void operator()(PtrWrap<void*> i) const {
        (*m_Stream) << std::hex << "0x" << i.getVal() << std::dec;
    }
    void operator()(PtrWrap<const void*> i) const {
        (*m_Stream) << std::hex << "0x" << i.getVal() << std::dec;
    }
    void operator()(GLenumWrap i) const {    //TODO: Remobe GLenumWrap. Not needed.
        (*m_Stream) << GetGLEnumName(i.get(), m_EnumGroup);
    }

    std::ostringstream* m_Stream;

    GLEnumGroup m_EnumGroup; //TODO: a stupid stateful hack. Remove it, or sanitize.
};

void AnyValue::writeToSS(std::ostringstream& out, GLEnumGroup enumGroup) const {
    boost::apply_visitor(AnyValueWriter(enumGroup, out), m_value);
}

std::string CalledEntryPoint::toString() const {
    std::ostringstream ret;
    ret << GetEntryPointName(m_entryp) << "(" << std::showpoint;

    for (size_t i = 0; i < m_args.size(); i++) {
        m_args[i].writeToSS(ret, GetEntryPointParamEnumGroup(m_entryp, i));
        if (i != m_args.size() - 1) {
            ret << ", ";
        }
    }

    ret << ")";

    if (m_retVal.isSet()) {
        ret << " = ";
        m_retVal.writeToSS(ret, GLEnumGroup::None); //TODO: None is just wrong here..
    }

    return ret.str();
}

const RetValue& CalledEntryPoint::getRetVal() const { return m_retVal; }

gl_t CalledEntryPoint::getError() const { return m_glError; }

const std::string& CalledEntryPoint::getDebugOutput() const {
    return m_DebugOutput;
}
