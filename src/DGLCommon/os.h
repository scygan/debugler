#ifndef OS_H
#define OS_H

#include <string>

class OsIcon {
public:
    virtual void * get() = 0;
};

class OsStatusPresenter {
public:
    virtual void setStatus(const std::string message) = 0;
    virtual ~OsStatusPresenter() {}
};

class Os {
public:
    static std::string getProcessName(); 

    static std::string getEnv(const char* variable);

    static void terminate();

    static OsStatusPresenter* createStatusPresenter();

    static OsIcon* createIcon();
};



#endif

