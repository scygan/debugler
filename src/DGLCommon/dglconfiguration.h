#ifndef DGLCONFIGURATION_H
#define DGLCONFIGURATION_H

class DGLConfiguration {
public:
    DGLConfiguration():m_BreakOnGLError(true), m_BreakOnDebugOutput(true) {}
    bool m_BreakOnGLError;
    bool m_BreakOnDebugOutput;
};

#endif