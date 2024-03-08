//
// Created by Serge on 05.03.2024.
//

#ifndef TOTALLY_NOT_RABBITMQ_SERVER_H
#define TOTALLY_NOT_RABBITMQ_SERVER_H

#include "protocol/STIP.h"
#include <boost/asio.hpp>
#include "protocol/Connection.h"
using boost::asio::ip::udp;

class STIPServer : public STIP_Base {
public:
    explicit STIPServer(udp::socket &socket);
    void accept();
private:
    ConnectionManager *connectionManager;
};


#endif //TOTALLY_NOT_RABBITMQ_SERVER_H
