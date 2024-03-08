//
// Created by Serge on 05.03.2024.
//

#include "server.h"

STIPServer::STIPServer(udp::socket &socket) : STIP_Base(socket) {
    this->socket = &socket;
    this->connectionManager = new ConnectionManager(socket);
}

void STIPServer::accept() {
    while (true) {
        for (;;) {
            STIP_PACKET packet[1];
            boost::system::error_code error;
            udp::endpoint remote_endpoint;
            size_t length = this->socket->receive_from(boost::asio::buffer(packet), remote_endpoint, 0, error);
            if (error && error != boost::asio::error::message_size) {
                throw boost::system::system_error(error);
            }
            std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;
            this->connectionManager->accept(remote_endpoint, packet[0]);
        }
    }
}



// TODO: Maybe rewrite to async

