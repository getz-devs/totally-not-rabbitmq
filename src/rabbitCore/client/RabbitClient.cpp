//
// Created by Serge on 23.03.2024.
//

#include "RabbitClient.h"

#include <utility>
#include "protocol/STIP.h"
#include "client/STIPClient.h"
#include "protocol/Connection.h"

using namespace STIP;

using boost::asio::ip::udp;

RabbitClient::RabbitClient(std::string host, int port) {
    this->host = std::move(host);
    this->port = port;
}

void RabbitClient::init() {
    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), host, std::to_string(port)).begin();

    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
    server_socket->open(udp::v4());

    STIPClient client(*server_socket);
    client.startListen();

    Connection *connection = client.connect(server_endpoint);
}

void receiveResutls() {

}