#ifndef OS_H
#define OS_H

#include <string>

#ifdef _WIN32
#define NO_RETURN __declspec(noreturn) 
#else
#define NO_RETURN __attribute__((noreturn))
#endif

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

    static void setEnv(const char* variable, const char* value);

    NO_RETURN static void fatal(const std::string& message);

    NO_RETURN static void terminate();

    static OsStatusPresenter* createStatusPresenter();

    static OsIcon* createIcon();

    static void setCurrentModuleHandle(void * handle);
private:
    static void* m_CurrentHandle;
};



#endif

