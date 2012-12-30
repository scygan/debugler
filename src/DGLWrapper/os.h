#ifndef OS_H
#define OS_H

#include <string>

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
};



#endif

