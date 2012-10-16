#include "transport.h"


namespace dglnet {

    Transport::Transport():m_socket(m_io_service), m_connected(false) {}

}