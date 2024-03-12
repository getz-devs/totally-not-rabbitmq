//
// Created by Serge on 07.03.2024.
//

#ifndef RABBIT_STIP_H
#define RABBIT_STIP_H


#include <iostream>
#include <boost/asio.hpp>
#include <random>

static const int MAX_UDP_SIZE = 65507 - 8 - 20 - 8;

struct STIP_HEADER {
    int command;
    uint32_t session_id;
    int size;
    int packet_id;

    // generate session id
};

uint32_t generate_session_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);
    return dis(gen);
}

static const int MAX_STIP_DATA_SIZE = MAX_UDP_SIZE - sizeof(STIP_HEADER);

struct STIP_PACKET {
    STIP_HEADER header;
    char data[MAX_STIP_DATA_SIZE];
};


using boost::asio::ip::udp;

class STIP_Base {
public:
    STIP_Base(udp::socket &socket);

    int ping(udp::endpoint &endpoint);

    void send_message(void *packet, int size, udp::endpoint &endpoint);
protected:
    void send_element_packet(void *packet, int size);

    udp::socket *socket;

    void send_element_packet(STIP_PACKET *packet, udp::endpoint &endpoint);


};


#endif //RABBIT_STIP_H
