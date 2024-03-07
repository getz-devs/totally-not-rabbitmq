//
// Created by Serge on 07.03.2024.
//


#include "main.h"


#include <iostream>
#include <boost/asio.hpp>

#include "../protocol/STIP.h"






int main() {
    try {
        boost::asio::io_context io_context;

        udp::resolver resolver(io_context);
        udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), "localhost", "12345").begin();


        udp::socket socket(io_context);
        socket.open(udp::v4());

        int datasize = 12;
        STIP_PACKET packet[1] = {};
        packet[0].header.command = 0x01;
        packet[0].header.packet_id = 0x03;
        packet[0].header.size = sizeof(STIP_HEADER)+datasize;
        std::string data = "I'm Ilya";
        std::copy(data.begin(), data.end(), packet[0].data);

        void* packet_ptr = &packet;
        std::cout << "Packet size: " << packet[0].header.size << std::endl;
        socket.send_to(boost::asio::buffer(packet_ptr,packet[0].header.size), server_endpoint);

        std::array<char, 128> recv_buffer;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buffer), sender_endpoint);
        std::cout.write(recv_buffer.data(), len);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
