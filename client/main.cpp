//
// Created by Serge on 07.03.2024.
//


#include "main.h"


#include <iostream>
#include <boost/asio.hpp>

#include "../protocol/STIP.h"


using boost::asio::ip::udp;



int main() {
    try {
        std::cout << "MAX_UDP_SIZE: " << MAX_UDP_SIZE << std::endl;
        std::cout << "MAX_STIP_DATA_SIZE: " << MAX_STIP_DATA_SIZE << std::endl;

        boost::asio::io_context io_context;

        udp::resolver resolver(io_context);
        udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12345").begin();


        udp::socket socket(io_context);
        socket.open(udp::v4());


        STIP_PACKET packet[1] = {};
        packet[0].header.command = 0x01;
        packet[0].header.offset = 0x02;
        packet[0].header.packet_id = 0x03;
        socket.send_to(boost::asio::buffer(packet), server_endpoint);

        std::array<char, 128> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);
        std::cout.write(recv_buffer.data(), len);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
