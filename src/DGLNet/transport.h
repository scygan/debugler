#include "message.h"


namespace dglnet {
    class Transport {
public: 
    Transport();
    void sendMessage(const Message& msg);

protected:
    boost::asio::io_service io_service; 

    
};


}


