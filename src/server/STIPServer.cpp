//
// Created by Serge on 05.03.2024.
//

#include "STIPServer.h"

namespace STIP {
    STIPServer::STIPServer(udp::socket &socket) {
        this->socket = &socket;
        this->connectionManager = new ConnectionManager(socket);
    }

    Connection *STIPServer::acceptConnection() {
        for (;;) {
            STIP_PACKET packet[1];
            boost::system::error_code error;
            udp::endpoint remote_endpoint;
            size_t length = this->socket->receive_from(boost::asio::buffer(packet), remote_endpoint, 0, error);
            if (error && error != boost::asio::error::message_size || length == 0) {
                return nullptr;
//            throw boost::system::system_error(error);
            }
            std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port()
                      << std::endl;

            std::cout << "Command: " << packet[0].header.command << "\n" << std::endl;

            STIP_PACKET response[1] = {};
            Connection *connection = nullptr;
            switch (packet[0].header.command) {
                case 100: // SYN
                    connection = new Connection(remote_endpoint, socket);
                    this->connectionManager->addConnection(remote_endpoint, connection);
                    response[0].header.command = 101; // SYN-ACK
                    response[0].header.size = sizeof(int); // We have to send only command
                    this->socket->send_to(boost::asio::buffer(response, response[0].header.size), remote_endpoint);
                    std::cout << "SYN-ACK sent" << std::endl;
                    break;

                    Connection *connection_exist;
                case 102: // ACK
                    connection_exist = this->connectionManager->getConnection(remote_endpoint);
                    if (connection_exist == nullptr) {
                        break;
                    }
                    connection_exist->setConnectionStatus(102);
                    connection_exist->startProcessing();
                    return connection_exist;
                default:
                    this->connectionManager->accept(remote_endpoint, packet[0]);
                    break;
            }
        }
    }

}

// TODO: Maybe rewrite to async read write from asio

