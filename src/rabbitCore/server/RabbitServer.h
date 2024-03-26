//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RABBITSERVER_H
#define RABBIT_RABBITSERVER_H

#include <boost/asio.hpp>
#include <sqlite_orm/sqlite_orm.h>
#include "protocol/Connection.h"

using boost::asio::ip::udp;
using namespace sqlite_orm;

class RabbitServer {
    public:
        RabbitServer(int port);

        void init();
        void startPolling();

        void processConnection(STIP::Connection *connection);

        ~RabbitServer() {
            delete server_socket;
        }
    private:
        int port;
        boost::asio::io_context io_context;
        udp::socket* server_socket;

};


#endif //RABBIT_RABBITSERVER_H
