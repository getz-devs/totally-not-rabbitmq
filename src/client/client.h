//
// Created by Serge on 07.03.2024.
//

#ifndef RABBIT_CLIENT_H
#define RABBIT_CLIENT_H

#include "protocol/Connection.h"
#include "protocol/STIP.h"


class STIPClient : public STIP_Base {
public:
    explicit STIPClient(udp::socket &socket, udp::endpoint &targetEndpoint);
    void send(const STIP_PACKET &packet, const udp::endpoint &endpoint);
    void receive();
private:
    udp::socket *socket;
    std::queue<STIP_PACKET> packetQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread mainThread;
};


#endif //RABBIT_CLIENT_H
