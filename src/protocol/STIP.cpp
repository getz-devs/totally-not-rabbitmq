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

void STIP_Base::send_message(void *packet, int size) {
//    return 0;
}

void STIP_Base::send_element_packet(void *packet, int size) {
//    return 0;
}