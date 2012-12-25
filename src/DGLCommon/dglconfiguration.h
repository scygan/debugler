#ifndef DGLCONFIGURATION_H
#define DGLCONFIGURATION_H

class DGLConfiguration {
public:
    DGLConfiguration():m_BreakOnGLError(true), m_BreakOnDebugOutput(true), m_BreakOnCompilerError(true) {}
    DGLConfiguration(bool breakOnGLError, bool breakOnDebugOutput, bool breakOnCompilerError):m_BreakOnGLError(breakOnGLError),
        m_BreakOnDebugOutput(breakOnDebugOutput), m_BreakOnCompilerError(breakOnCompilerError) {}
    bool m_BreakOnGLError;
    bool m_BreakOnDebugOutput;
    bool m_BreakOnCompilerError;
};

#endif