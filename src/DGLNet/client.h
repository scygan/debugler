#include<string>

#include "transport.h"


namespace dglnet {

class Client: public Transport {
public: 
    Client(std::string host, std::string port);
    
private:
    void setStatus(std::string) {}
};

}//namespace dglnet