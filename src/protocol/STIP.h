//
// Created by Serge on 07.03.2024.
//

#ifndef RABBIT_STIP_BASE_H
#define RABBIT_STIP_BASE_H


#include <iostream>
#include <boost/asio.hpp>
#include <random>

static const int MAX_UDP_SIZE = 65507 - 8 - 20 - 8;

struct STIP_HEADER {
    int command;
    uint32_t session_id;
    int size;
    uint32_t packet_id;

    // generate session id
};


static const int MAX_STIP_DATA_SIZE = MAX_UDP_SIZE - sizeof(STIP_HEADER);

struct STIP_PACKET {
    STIP_HEADER header;
    char data[MAX_STIP_DATA_SIZE];
};


using boost::asio::ip::udp;


#endif //RABBIT_STIP_BASE_H
