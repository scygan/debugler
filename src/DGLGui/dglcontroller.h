#include <QObject>

#include "DGLNet/client.h"
#include <boost/make_shared.hpp>

class DglController: public QObject {
    Q_OBJECT

public:
    void connect(const std::string& host, const std::string& port);

signals:
    void disconnected();
    void connected();

    void breaked();
    void running();
    

private:
    boost::shared_ptr<dglnet::Client> m_DglClient;
};