//
// Created by Serge on 05.03.2024.
//

#ifndef TOTALLY_NOT_RABBITMQ_SERVER_H
#define TOTALLY_NOT_RABBITMQ_SERVER_H


//#include <boost/asio.hpp>

#include "protocol/Connection.h"

//using boost::asio::ip::udp;

class STIPServer {
public:
    explicit STIPServer(udp::socket &socket);

    Connection *acceptConnection();

private:
    udp::socket *socket;
    ConnectionManager *connectionManager;
};


#endif //TOTALLY_NOT_RABBITMQ_SERVER_H
