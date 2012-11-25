#ifndef DGLCONFIGURATION_H
#define DGLCONFIGURATION_H

class DGLConfiguration {
public:
    DGLConfiguration():m_BreakOnGLError(true) {}
    bool m_BreakOnGLError;
};

#endif