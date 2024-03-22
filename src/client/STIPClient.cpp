//
// Created by Serge on 07.03.2024.
//


#include "STIPClient.h"

STIPClient::STIPClient(udp::socket &socket) {
    this->socket = &socket;
    this->connectionManager = new ConnectionManager(socket);
}


void STIPClient::receiveProcess() {
    std::cout << "Start listen" << std::endl;
    while (isRunning) {
        STIP_PACKET packet[1];
        boost::system::error_code error;
        udp::endpoint remote_endpoint;
        size_t length = socket->receive_from(boost::asio::buffer(packet), remote_endpoint, 0, error);
        if (error && error != boost::asio::error::message_size) {
            return;
//            throw boost::system::system_error(error);
        }
        std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;

        connectionManager->accept(remote_endpoint, packet[0]);
    }
}


Connection *STIPClient::connect(udp::endpoint &targetEndpoint) {
    // TODO: Add error handling

    auto *connection = new Connection(targetEndpoint, socket);
    this->connectionManager->addConnection(targetEndpoint, connection);
    STIP_PACKET packet[1] = {};
    packet[0].header.command = 100;
    packet[0].header.size = sizeof(int);

    this->socket->send_to(boost::asio::buffer(packet, packet[0].header.size), targetEndpoint);
    bool result = false;
    STIP_PACKET response = connection->getPacket(result);

    // TODO: Add better error handling
    if (!result) {
        throw std::runtime_error("Error while waiting for response");
    }
    if (response.header.command == 101) {
        packet[0].header.command = 102;
        packet[0].header.size = sizeof(int);
        this->socket->send_to(boost::asio::buffer(packet, packet[0].header.size), targetEndpoint);
        connection->setConnectionStatus(102);
        connection->startProcessing();
        return connection;
    }
    throw std::runtime_error("Error while connecting");
}

void STIPClient::startListen() {
    if (this->isRunning) {
        return;
    }
    this->isRunning = true;
    this->mainThread = std::thread(&STIPClient::receiveProcess, this);
}

void STIPClient::stopListen() {
    if (!this->isRunning) {
        return;
    }
    this->isRunning = false;
    socket->cancel();
    this->mainThread.join();
}