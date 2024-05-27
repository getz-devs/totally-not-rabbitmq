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

private:
    int port;
    std::string id;
    std::string host;

    STIP::STIPClient *client;
    STIP::Connection *connection{};

    udp::socket *server_socket{};
    udp::resolver *resolver;
    udp::endpoint *server_endpoint;

    boost::asio::io_context io_context;
};

#endif //RABBIT_RabbitClient_H
