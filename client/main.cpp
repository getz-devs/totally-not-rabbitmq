//
// Created by Serge on 07.03.2024.
//


#include "main.h"


#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main() {
    try {
        boost::asio::io_context io_context;
        udp::resolver resolver(io_context);
        udp::endpoint receiver_endpoint = *resolver.resolve(udp::v4(), "localhost", "12345").begin();

        udp::socket socket(io_context);
        socket.open(udp::v4());

        std::string message = "Ping";
        socket.send_to(boost::asio::buffer(message), receiver_endpoint);

        std::array<char, 128> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);
        std::cout.write(recv_buffer.data(), len);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
