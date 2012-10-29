#include "gl-serialized.h"


CalledEntryPoint::CalledEntryPoint(Entrypoint entryp, int numArgs):m_entryp(entryp), m_SavedArgsCount(0) {
    m_args.resize(numArgs);
}

Entrypoint CalledEntryPoint::getEntrypoint() const { return m_entryp; }

const std::vector<AnyValue>& CalledEntryPoint::getArgs() const{
    return m_args;
}