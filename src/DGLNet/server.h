#include "transport.h"


namespace dglnet {

class Server: public Transport {
public: 
    Server(int port);
protected:
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::asio::ip::tcp::acceptor m_acceptor;
};

}