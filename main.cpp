//
// Created by Serge on 05.03.2024.
//
#include <boost/asio.hpp>
#include <iostream>

enum class Command : uint8_t {
    InitConnection = 0,
    GoodAnswer = 1,
    BadAnswer = 2,
    SendData = 3,
    Ask = 4
};

void handle_command(Command cmd) {
    switch (cmd) {
        case Command::InitConnection:
            std::cout << "Handle InitConnection\n";
            break;
        case Command::GoodAnswer:
            std::cout << "Handle GoodAnswer\n";
            break;
        case Command::BadAnswer:
            std::cout << "Handle BadAnswer\n";
            break;
        case Command::SendData:
            std::cout << "Handle SendData\n";
            break;
        case Command::Ask:
            std::cout << "Handle Ask\n";
            break;
    }
}

int main() {
    boost::asio::io_service io_service;
    boost::asio::ip::udp::socket socket(io_service);
    boost::asio::ip::udp::endpoint remote_endpoint;

    socket.open(boost::asio::ip::udp::v4());

    for (;;) {
        char recv_buf[1];
        socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
        handle_command(static_cast<Command>(recv_buf[0]));
    }

    return 0;
}
//#include "main.h"
