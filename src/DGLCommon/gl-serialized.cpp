#include "gl-serialized.h"

#include <sstream>

CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, int numArgs):m_entryp(entryp), m_SavedArgsCount(0) {
    m_args.resize(numArgs);
}

Entrypoint CalledEntryPoint::getEntrypoint() const { return m_entryp; }

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
        (*m_Stream) << reinterpret_cast<int>((const void*)i);
    }
    void operator()(PtrWrap<const void*> i) const {
        (*m_Stream) << reinterpret_cast<int>((const void*)i);
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
    return ret.str();;
}