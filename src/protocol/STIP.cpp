//
// Created by Serge on 07.03.2024.
//

#include "STIP.h"


//class STIP_Base {
////
////
//};
STIP_Base::STIP_Base(udp::socket &socket) {
    this->socket = &socket;
}

void STIP_Base::send_message(void *packet, int size, udp::endpoint &endpoint) {
    // splitting to packets
    int packet_count = size / MAX_STIP_DATA_SIZE + 1;
    for (int i = 0; i < packet_count; i++) {
        int packet_size = MAX_STIP_DATA_SIZE;
        if (i == packet_count - 1) {
            packet_size = size - i * MAX_STIP_DATA_SIZE;
        }
        STIP_PACKET *stip_packet = (STIP_PACKET *) malloc(sizeof(STIP_PACKET));
        stip_packet->header.command = 0;
        stip_packet->header.size = packet_size;
        stip_packet->header.packet_id = i;
        memcpy(stip_packet->data, (char *) packet + i * MAX_STIP_DATA_SIZE, packet_size);
//        send_element_packet(stip_packet, sizeof(STIP_PACKET));
    }
}

int STIP_Base::ping(udp::endpoint &endpoint) {
    STIP_PACKET packet[1] = {};
    packet[0].header.command = 10; // Ping command
    packet[0].header.packet_id = 0x03;
    packet[0].header.size = sizeof(STIP_HEADER);
    packet[0].header.session_id = generate_session_id();
//    float hash = 123.5;
    send_element_packet(packet, endpoint);
//    STIP_PACKET = wait_result(endpoint, hash, 60);
//    wait_result(endpoint);



    return 0;
}

void STIP_Base::send_element_packet(STIP_PACKET *packet, udp::endpoint &endpoint) {
    this->socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
}
