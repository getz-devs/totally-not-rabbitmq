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
#include "protocol/STIP.h"
using boost::asio::ip::udp;

class Connection {
private:

    std::mutex mtx;
    std::condition_variable cv;
    Timer timeoutTimer;
    std::thread mainThread;
    udp::endpoint endpoint;
    udp::socket *socket = nullptr;
    char connectionStatus = 100;
    std::queue<STIP_PACKET> packetQueue;

public:
    Connection(udp::endpoint &endpoint, udp::socket *socket);

    void sendData(void* data, size_t size);

    void addPacket(const STIP_PACKET &packet);

    STIP_PACKET getPacket();

    void setConnectionStatus(char status);

    ~Connection();
};


class ConnectionManager {
private:
    std::unordered_map<udp::endpoint, Connection *> connections;
    std::mutex mtx;
    const udp::socket *socket;

public:
    explicit ConnectionManager(const udp::socket &socket);

//    ConnectionManager(const udp::endpoint &endpoint, udp::socket &socket);
    void accept(const udp::endpoint &endpoint, const STIP_PACKET &packet);

    void addConnection(const udp::endpoint &endpoint, Connection *connection);

    bool check(const udp::endpoint &endpoint);

    Connection* getConnection(const udp::endpoint &endpoint);

    void remove(const udp::endpoint &endpoint);

    ~ConnectionManager();
};


#endif //RABBIT_CONNECTION_H
