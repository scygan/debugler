#include "gl-serialized.h"

#include <sstream>
#include <iomanip>
#include <cstring>

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
    AnyValueWriter(std::ostringstream& stream):m_Stream(&stream) {}

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
        (*m_Stream) << std::hex << "0x" << (const void*)i << std::dec;
    }
    void operator()(PtrWrap<const void*> i) const {
        (*m_Stream) << std::hex << "0x" << (const void*)i << std::dec;
    }
    void operator()(GLenumWrap i) const {
        (*m_Stream) << GetGLEnumName(i.get());
    }

    std::ostringstream * m_Stream;
};

void AnyValue::writeToSS(std::ostringstream& out) const {
    boost::apply_visitor(AnyValueWriter(out), m_value);
}

std::string CalledEntryPoint::toString() const {
    std::ostringstream ret;
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

ContextObjectName::ContextObjectName():m_Context(0), m_Name(0), m_Target(0) {}
ContextObjectName::ContextObjectName(uint32_t context, uint32_t name, uint32_t target):m_Name(name),m_Context(context),m_Target(target) {}
ContextObjectName::~ContextObjectName() {}

 bool ContextObjectName::operator==(const ContextObjectName&rhs) const {

    //it is crucial that m_Target is not get into account here (ID + ctxID is enough to indentify an object and m_Target is optional)

    return m_Context == rhs.m_Context && m_Name == rhs.m_Name;
}

bool ContextObjectName::operator<(const ContextObjectName&rhs) const {

    //it is crucial that m_Target is not get into account here (ID + ctxID is enough to indentify an object and m_Target is optional)

    if (m_Context < rhs.m_Context)
        return true;
    if (m_Context > rhs.m_Context)
        return false;
    if (m_Name < rhs.m_Name)
        return true;
    return false;
}

DGLPixelRectangle::DGLPixelRectangle(int32_t width, int32_t height, int32_t rowBytes, uint32_t glFormat, uint32_t glType, uint32_t iFormat, int32_t samples):m_Width(width),
    m_Height(height), m_RowBytes(rowBytes), m_GLFormat(glFormat), m_GLType(glType), m_InternalFormat(iFormat), m_Samples(samples), m_Storage(NULL) {

        if (m_Height * m_RowBytes) {
            m_Storage = malloc(m_Height * m_RowBytes);
        }
}

DGLPixelRectangle::DGLPixelRectangle(const DGLPixelRectangle& rhs):m_Width(rhs.m_Width), m_Height(rhs.m_Height),
    m_RowBytes(rhs.m_RowBytes), m_GLFormat(rhs.m_GLFormat), m_GLType(rhs.m_GLType), m_InternalFormat(rhs.m_InternalFormat), m_Samples(rhs.m_Samples) {
    if (rhs.getPtr()) {
        m_Storage = malloc(m_Height * m_RowBytes);
        memcpy(m_Storage, rhs.getPtr(), m_Height * m_RowBytes);
    } else {
        m_Storage = NULL;
    }
}

DGLPixelRectangle::~DGLPixelRectangle() {
    if (m_Storage) {
        free(m_Storage);
        m_Storage = NULL;
    }
}

void* DGLPixelRectangle::getPtr() const {
    return m_Storage;
}

size_t DGLPixelRectangle::getSize() const {
    return m_Height * m_RowBytes;
}
