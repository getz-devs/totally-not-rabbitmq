//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RABBITSERVER_H
#define RABBIT_RABBITSERVER_H

#include <boost/asio.hpp>

using boost::asio::ip::udp;


class RabbitServer {
    public:
        RabbitServer(int port);

        void init();
        void startPolling();
    private:
        int port;
        boost::asio::io_context io_context;
        udp::socket server_socket;
};


#endif //RABBIT_RABBITSERVER_H
