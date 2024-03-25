//
// Created by Serge on 23.03.2024.
//
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "RabbitServer.h"

using namespace STIP;

using boost::asio::ip::udp;



RabbitServer::RabbitServer(int port) {
    this->port = port;
    this->server_socket = udp::socket(io_context, udp::endpoint(udp::v4(), port));
}

void RabbitServer::init() {
    boost::asio::io_context io_context;

    // Создаем UDP сокет для приема запросов на порту 12345

}

void RabbitServer::startPolling() {
    STIPServer server(server_socket);

    for (;;) {
        Connection *connection = server.acceptConnection();
        std::cout << "Connection accepted\n\n" << std::endl;
        auto *clientHandler = new ClientHandler();
        std::thread(&ClientHandler::processConnection, clientHandler, connection).detach();
    }
}
