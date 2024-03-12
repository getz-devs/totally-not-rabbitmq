//
// Created by Serge on 07.03.2024.
//


#include "client.h"

STIPClient::STIPClient(udp::socket &socket, udp::endpoint &targetEndpoint) : STIP_Base(socket) {
    this->socket = &socket;
//    this->mainThread = std::thread(&STIPClient::receive, this);
}

void STIPClient::send(const STIP_PACKET &packet, const udp::endpoint &endpoint) {

}

void STIPClient::receive() {
    while (true) {
        STIP_PACKET packet[1];
        boost::system::error_code error;
        udp::endpoint remote_endpoint;
        size_t length = this->socket->receive_from(boost::asio::buffer(packet), remote_endpoint, 0, error);
        if (error && error != boost::asio::error::message_size) {
            throw boost::system::system_error(error);
        }
        std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;
    }
}


