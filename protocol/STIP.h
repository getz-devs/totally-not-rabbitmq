//
// Created by Serge on 07.03.2024.
//

#ifndef RABBIT_STIP_H
#define RABBIT_STIP_H


#include <iostream>
#include <boost/asio.hpp>

static const int MAX_UDP_SIZE = 65507 - 8 - 20 - 8;

struct STIP_HEADER {
    int command;
    int size;
    int packet_id;
};

static const int MAX_STIP_DATA_SIZE = MAX_UDP_SIZE - sizeof(STIP_HEADER);

struct STIP_PACKET {
    STIP_HEADER header;
    char data[MAX_STIP_DATA_SIZE];
};


using boost::asio::ip::udp;

class STIP_Base {
public:
    STIP_Base(udp::socket &socket);

    void send_message(void *packet, int size);
private:
    void send_element_packet(void *packet, int size);

    udp::socket *socket;
};


#endif //RABBIT_STIP_H
