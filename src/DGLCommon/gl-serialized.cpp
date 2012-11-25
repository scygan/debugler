#include "gl-serialized.h"

#include <sstream>

CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, int numArgs):m_entryp(entryp), m_SavedArgsCount(0), m_glError(GL_NO_ERROR) {
    m_args.resize(numArgs);
}

Entrypoint CalledEntryPoint::getEntrypoint() const { return m_entryp; }

void CalledEntryPoint::setError(uint32_t error) {
    m_glError = error;
}

const std::vector<AnyValue>& CalledEntryPoint::getArgs() const{
    return m_args;
}


class AnyValueWriter: public boost::static_visitor<void> {
public:
    AnyValueWriter(std::stringstream& stream):m_Stream(&stream) {}

    void operator()(signed long long i) const { (*m_Stream) << i; }
    void operator()(unsigned long long i) const { (*m_Stream) << i; }
    void operator()(signed long i) const { (*m_Stream) << i; }
    void operator()(unsigned long i) const { (*m_Stream) << i; }
    void operator()(unsigned int i) const { (*m_Stream) << i; }
    void operator()(signed int i) const { (*m_Stream) << i; }
    void operator()(unsigned short i) const { (*m_Stream) << i; }
    void operator()(signed short i) const { (*m_Stream) << i; }
    void operator()(unsigned char i) const { (*m_Stream) << i; }
    void operator()(signed char i) const { (*m_Stream) << i; }
    void operator()(float f) const { (*m_Stream) << f; }
    void operator()(double d) const { (*m_Stream) << d; }
    void operator()(PtrWrap<void*> i) const {
        (*m_Stream) << std::hex << "0x" << reinterpret_cast<int>((const void*)i) << std::dec;
    }
    void operator()(PtrWrap<const void*> i) const {
        (*m_Stream) << std::hex << "0x" << reinterpret_cast<int>((const void*)i) << std::dec;
    }
    void operator()(GLenumWrap i) const {
        (*m_Stream) << GetGLEnumName(i.get());
    }

    std::stringstream * m_Stream;
};

std::string AnyValue::toString() const {
    std::stringstream ret;
    boost::apply_visitor(AnyValueWriter(ret), m_value);
    return ret.str();
}

std::string CalledEntryPoint::toString() const {
    std::stringstream ret;
    ret << GetEntryPointName(m_entryp) << "(";

    for (size_t i = 0; i < m_args.size(); i++) {
        ret << m_args[i].toString();
        if (i != m_args.size() - 1) {
            ret << ", ";
        }
    }

    ret << ")";
    if (m_glError != GL_NO_ERROR) {
        ret << " -> " << GetGLEnumName(m_glError);
    }
    return ret.str();;
}