#include<string>

#include "transport.h"


namespace dglnet {

class IController {
public:
    virtual void onSetStatus(std::string) = 0;
    virtual void onInternalError(std::string) = 0;

};


class Client: public Transport {
public: 
    Client(std::string host, std::string port, IController* controller);
    int getSocketFD();
    
    void poll();

private:
    void onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void onConnect(const boost::system::error_code &err);

    IController* m_controller;
};

}//namespace dglnet