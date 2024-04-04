//
// Created by Serge on 23.03.2024.
//

#include "RabbitClient.h"

#include <utility>
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"

using namespace STIP;

using boost::asio::ip::udp;

RabbitClient::RabbitClient(std::string host, int port) {
    this->host = std::move(host);
    this->port = port;
}

void RabbitClient::init() {
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
}
