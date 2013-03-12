#ifndef DGLPROCESS_H
#define DGLPROCESS_H


#include<string>

class DGLProcess {
public:
    virtual ~DGLProcess() {}
    virtual bool waitReady(int msec) = 0;

    static DGLProcess* Create(std::string cmd, std::string path, std::string args, int port, bool modeEGL);
};






#endif //DGLPROGRAMVIEW_H