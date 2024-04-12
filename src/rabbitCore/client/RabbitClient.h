//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RabbitClient_H
#define RABBIT_RabbitClient_H

#include <boost/asio.hpp>
#include "protocol/Connection.h"
#include <nlohmann/json.hpp>

using boost::asio::ip::udp;
using json = nlohmann::json;

class RabbitClient {
public:
    RabbitClient(std::string host, int port);

    void init();

    // - нужно?
    // void startPolling();
    // void processConnection(STIP::Connection *connection);
    //

    void receiveResutls();

    ~RabbitClient() {
        delete server_socket;
    }

private:
    std::string host;
    int port;

    udp::socket *server_socket{};

//        json parseMessage(std::string message);
//    static bool validateRequest(json request);


};


#endif //RABBIT_RabbitClient_H
