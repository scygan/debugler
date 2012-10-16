#include<string>

#include "transport.h"


namespace dglnet {

class Client: public Transport {
public: 
    Client(std::string host, std::string port);
    
private:
    void setStatus(const std::string&) {}
    void onResolve(const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void onConnect(const boost::system::error_code &ec);
};

}//namespace dglnet