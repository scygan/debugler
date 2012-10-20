#include "transport.h"


namespace dglnet {

    Transport::Transport():m_socket(m_io_service), m_connected(false) {}
    
    void Transport::poll() {
        while (m_io_service.poll());
    }

    void Transport::sendMessage(const Message* msg) {

    }

}