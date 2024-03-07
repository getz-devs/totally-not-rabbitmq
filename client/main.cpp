//
// Created by Serge on 07.03.2024.
//


#include "main.h"


#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::udp;

int main() {
    try {
        boost::asio::io_service io_service;

        udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));

        udp::resolver resolver(io_service);
        udp::endpoint endpoint = *resolver.resolve({udp::v4(), "localhost", "daytime"});
        socket.connect(endpoint);

        for (;;) {
            std::array<char, 1> send_buf  = {{ 0 }};
            socket.send(boost::asio::buffer(send_buf));

            std::array<char, 128> recv_buf;
            udp::endpoint sender_endpoint;
            size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

            std::cout.write(recv_buf.data(), len);

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}