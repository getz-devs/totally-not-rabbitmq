//
// Created by Serge on 08.03.2024.
//

#ifndef RABBIT_CONNECTION_H
#define RABBIT_CONNECTION_H


#include <queue>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include "protocol/STIP.h"
#include "utils/Timer.h"
#include "protocol/Connection.h"

class Session {
public:
     Session() = default;
};


class ConnectionManager;
class EndpointConnection {
private:
    std::queue<STIP_PACKET> packetQueue;
    std::mutex mtx;
    std::condition_variable cv;
    Timer timeoutTimer;
    std::thread mainThread;
    udp::endpoint endpoint;
    const udp::socket *socket;
    ConnectionManager *connectionManager;
    std::unordered_map<uint32_t, Session> sessions;

public:
    EndpointConnection(udp::endpoint endpoint, const udp::socket &socket, ConnectionManager &connectionManager);

    void addPacket(const STIP_PACKET &packet);

    STIP_PACKET getPacket();
    ~EndpointConnection();
};


class ConnectionManager {
private:
    std::unordered_map<udp::endpoint, EndpointConnection *> connections;
    std::mutex mtx;
    const udp::socket *socket;

public:
    explicit ConnectionManager(const udp::socket &socket);

//    ConnectionManager(const udp::endpoint &endpoint, udp::socket &socket);
    void accept(const udp::endpoint &endpoint, const STIP_PACKET &packet);

    void remove(const udp::endpoint &endpoint);

    ~ConnectionManager();
};


#endif //RABBIT_CONNECTION_H
