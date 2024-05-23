//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RabbitClient_H
#define RABBIT_RabbitClient_H

#include <boost/asio.hpp>
#include "protocol/Connection.h"
#include <nlohmann/json.hpp>
#include "DataModel/TaskRequest.h"
#include "client/STIPClient.h"

using boost::asio::ip::udp;
using json = nlohmann::json;

class RabbitClient {
public:
    RabbitClient(std::string id, std::string host, int port);

    void init();
    void receiveResutls();
    void sendTask(TaskRequest t);

    ~RabbitClient() {
        delete server_socket;
    }

    void testMessage(std::string msg);

private:
    STIP::STIPClient *client;

    std::string id;
    std::string host;
    int port;
    STIP::Connection *connection{};
    udp::socket *server_socket{};

    udp::resolver *resolver;
    udp::endpoint *server_endpoint;

    boost::asio::io_context io_context;
};

#endif //RABBIT_RabbitClient_H
