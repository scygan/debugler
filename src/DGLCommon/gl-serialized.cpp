#include "gl-serialized.h"

#include <sstream>
#include <iomanip>

CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, int numArgs):m_entryp(entryp), m_SavedArgsCount(0), m_glError(GL_NO_ERROR) {
    m_args.resize(numArgs);
}

Entrypoint CalledEntryPoint::getEntrypoint() const { return m_entryp; }

void CalledEntryPoint::setError(uint32_t error) {
    m_glError = error;
}

void CalledEntryPoint::setDebugOutput(const std::string& message) {
    m_DebugOutput = message;
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
    void operator()(unsigned char i) const { (*m_Stream) << (unsigned int)i; }
    void operator()(signed char i) const { (*m_Stream) << (int) i; }
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

void AnyValue::writeToSS(std::stringstream& out) const {
    boost::apply_visitor(AnyValueWriter(out), m_value);
}

std::string CalledEntryPoint::toString() const {
    std::stringstream ret;
    ret << GetEntryPointName(m_entryp) << "(" << std::showpoint;

    for (size_t i = 0; i < m_args.size(); i++) {
        m_args[i].writeToSS(ret);
        if (i != m_args.size() - 1) {
            ret << ", ";
        }
    }

    ret << ")";
    return ret.str();;
}

GLenum CalledEntryPoint::getError() const {
    return m_glError;
}

const std::string& CalledEntryPoint::getDebugOutput() const {
    return m_DebugOutput;
}


DGLResourceFBO::FBOAttachment::FBOAttachment(uint32_t id):m_Ok(true),m_Id(id) {}

void DGLResourceFBO::FBOAttachment::error(std::string msg) {
    m_Ok = false;
    m_ErrorMsg = msg;
}

bool DGLResourceFBO::FBOAttachment::isOk(std::string& msg) const {
    msg = m_ErrorMsg;
    return m_Ok;
}