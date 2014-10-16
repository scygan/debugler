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
#include <DGLCommon/def.h>

#include <sstream>
#include <iomanip>
#include <cstring>

CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, size_t numArgs)
        : m_entryp(entryp), m_glError(GL_NO_ERROR) {
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
    AnyValueWriter(std::ostringstream& stream) : m_Stream(&stream) {}

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
   
    std::ostringstream* m_Stream;
};

template<typename T>
class AnyValueCaster : public boost::static_visitor<T> {
public:
    template<typename T2>
    T operator()(T2 i) const { return static_cast<T>(i); }
    T operator()(PtrWrap<void*> i) const {
        return static_cast<T>(i.getVal());
    }
    T operator()(PtrWrap<const void*> i) const {
        return static_cast<T>(i.getVal());
    }
};

void AnyValue::writeToSS(std::ostringstream& out, const GLParamTypeMetadata& paramMetadata) const {
    if (paramMetadata.m_BaseType == GLParamTypeMetadata::BaseType::Value) {
        
        boost::apply_visitor(AnyValueWriter(out), m_value);

    } else if (paramMetadata.m_BaseType == GLParamTypeMetadata::BaseType::Enum) {
        
        out << GetGLEnumName(
            boost::apply_visitor(AnyValueCaster<gl_t>(), m_value),
            paramMetadata.m_EnumGroup);

    } else if (paramMetadata.m_BaseType == GLParamTypeMetadata::BaseType::Bitfield) {
        value_t bitfield = boost::apply_visitor(AnyValueCaster<value_t>(), m_value);

        value_t j = 1;
        bool first = true;
        for (size_t i = 0; i < sizeof(bitfield) * 8; i++) {
            if (j & bitfield) {
                if (!first) {
                    out << " || ";
                } else {
                    first = false;
                }
                out << GetGLEnumName(j & bitfield, paramMetadata.m_EnumGroup);
            }
            j *= 2;
        }

    } else {
        DGL_ASSERT(0);
    }
}

std::string CalledEntryPoint::toString() const {
    std::ostringstream ret;
    ret << GetEntryPointName(m_entryp) << "(" << std::showpoint;

    for (size_t i = 0; i < m_args.size(); i++) {
        m_args[i].writeToSS(ret, GetEntryPointGLParamTypeMetadata(m_entryp, i));
        if (i != m_args.size() - 1) {
            ret << ", ";
        }
    }

    ret << ")";

    if (m_retVal.isSet()) {
        ret << " = ";
        m_retVal.writeToSS(ret, GetEntryPointRetvalMetadata(m_entryp));
    }

    return ret.str();
}

const RetValue& CalledEntryPoint::getRetVal() const { return m_retVal; }

gl_t CalledEntryPoint::getError() const { return m_glError; }

const std::string& CalledEntryPoint::getDebugOutput() const {
    return m_DebugOutput;
}
