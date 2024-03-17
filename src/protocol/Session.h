//
// Created by Serge on 16.03.2024.
//

#ifndef RABBIT_SESSION_H
#define RABBIT_SESSION_H

#include <iostream>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "protocol/STIP.h"
#include "protocol/STIPVersion.h"

//using namespace std;


class Session {
public:
    virtual void processIncomingPacket(STIP_PACKET packet) = 0;

    uint32_t getId() const;

protected:
    uint32_t id = 0;
    void *data = nullptr;
};

class PingSession : public Session {
public:
    PingSession() {
        std::cout << "PingSession created" << std::endl;
    }

    explicit PingSession(uint32_t id);

    void processIncomingPacket(STIP_PACKET packet) override;

    static void serverAnswer(udp::socket &socket, udp::endpoint &endpoint, uint32_t sessionId);

    ~PingSession();

    uint32_t waitAnswer();

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool isAnswered = false;
    uint32_t answer = 0;
};


class SessionManager {
public:
    void addSession(Session *session);

    void deleteSession(Session *session);

    Session *getSession(uint32_t id);

    uint32_t generateSessionId();

private:
    std::map<uint32_t, Session *> sessions;
    std::mutex mtx;
};


#endif //RABBIT_SESSION_H
