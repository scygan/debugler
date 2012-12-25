#ifndef DGLCONFIGURATION_H
#define DGLCONFIGURATION_H

class DGLConfiguration {
public:
    DGLConfiguration():m_BreakOnGLError(true), m_BreakOnDebugOutput(true), m_BreakOnCompilerError(true) {}
    bool m_BreakOnGLError;
    bool m_BreakOnDebugOutput;
    bool m_BreakOnCompilerError;
};

#endif