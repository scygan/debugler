#include<string>

#include "transport.h"


namespace dglnet {

class IController {
public:
    virtual void onSetStatus(std::string) = 0;
    virtual void onSocket() = 0;

    virtual ~IController() {}
};


class Client: public Transport {
public: 
    Client(IController* controller, MessageHandler* messageHandler);
    void connectServer(std::string host, std::string port);
    int getSocketFD();

private:
    void onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void onConnect(const boost::system::error_code &err);
    boost::shared_ptr<Client> shared_from_this();

    IController* m_controller;
};

}//namespace dglnet