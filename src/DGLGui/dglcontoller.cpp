#include "dglcontroller.h"





void DglController::connect(const std::string& host, const std::string& port) {
    if (m_DglClient) {
        disconnected();
    }
    m_DglClient = boost::make_shared<dglnet::Client>(host, port);
}