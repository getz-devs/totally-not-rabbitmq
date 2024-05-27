#ifndef TOTALLY_NOT_RABBITMQ_SERVER_H
#define TOTALLY_NOT_RABBITMQ_SERVER_H


//#include <boost/asio.hpp>

#include "protocol/Connection.h"

//using boost::asio::ip::udp;

namespace STIP {

    class STIPServer {
    public:
        explicit STIPServer(udp::socket &socket);

        Connection *acceptConnection();

    private:
        udp::socket *socket;
        ConnectionManager *connectionManager;
    };

}


#endif //TOTALLY_NOT_RABBITMQ_SERVER_H
