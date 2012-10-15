#include "transport.h"


namespace dglnet {

class Server: public Transport {
public: 
    Server(int port);
};

}