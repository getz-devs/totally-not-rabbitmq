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
    int packet_id;

    // generate session id
};


static const int MAX_STIP_DATA_SIZE = MAX_UDP_SIZE - sizeof(STIP_HEADER);

struct STIP_PACKET {
    STIP_HEADER header;
    char data[MAX_STIP_DATA_SIZE];
};


using boost::asio::ip::udp;

//class STIP_Base {
//public:
//    STIP_Base(udp::socket &socket);
//
//    int ping(udp::endpoint &endpoint);
//protected:
//    udp::socket *socket = nullptr;
//
////    session::SessionManager *sessionManager;
//    ConnectionManager *connectionManager = nullptr;
//};


#endif //RABBIT_STIP_H
