//
// Created by Serge on 07.03.2024.
//

#ifndef RABBIT_STIPCLIENT_H
#define RABBIT_STIPCLIENT_H

#include "protocol/Connection.h"
#include "protocol/STIP.h"

namespace STIP {

    class STIPClient {
    public:
        explicit STIPClient(udp::socket &socket);

        void startListen();

        void stopListen();

        Connection *connect(udp::endpoint &targetEndpoint);

    private:
        udp::socket *socket;
        std::queue<STIP_PACKET> packetQueue;
        std::mutex mtx;
        std::condition_variable cv;
        std::thread mainThread;
        ConnectionManager *connectionManager = nullptr;
        bool isRunning = false;

        void receiveProcess();
    };

}

#endif //RABBIT_STIPCLIENT_H
