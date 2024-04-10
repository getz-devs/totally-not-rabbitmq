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
#include <vector>
//using namespace std;
using boost::asio::ip::udp;

namespace STIP {

    class Session {
    public:
        virtual void processIncomingPacket(STIP_PACKET packet) = 0;

        uint32_t getId() const;

    protected:
        uint32_t id = 0;
        void *data = nullptr;
    };



// ------------------------------------------------ PingSession.h ------------------------------------------------

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



// ------------------------------------------------ SendMessageSession.h ------------------------------------------------

    class SendMessageSession : public Session {
    public:
        explicit SendMessageSession(uint32_t id, void *data, uint32_t size, udp::socket *socket,
                                    udp::endpoint &endpoint) {
            status = -1;
            this->id = id;
            this->data = data;
            this->size = size;

            this->socket = socket;
            this->endpoint = endpoint;

            packet_counts = size / MAX_STIP_DATA_SIZE + 1;
        }

        void processIncomingPacket(STIP_PACKET packet) override;

//    void sendAnswer(udp::socket &socket, udp::endpoint &endpoint, void *data, size_t size);
        bool initSend();

        bool sendData();

        bool waitApproval();

        void cancel();

    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool _cancaled = false;

        void *data = nullptr;
        size_t size = 0;
        size_t packet_counts = 0;
        int status = -1;

        udp::socket *socket = nullptr;
        udp::endpoint endpoint;

        void sendPart(size_t packet_id);
    };

// ------------------------------------------------ ReceiveMessageSession.h ------------------------------------------------

    class ReceiveMessageSession : public Session {
    public:
        explicit ReceiveMessageSession(uint32_t id, size_t size, size_t packet_counts, udp::socket *socket,
                                       udp::endpoint &endpoint) {
            status = -1;
            this->id = id;
            this->size = size;
            this->packet_counts = packet_counts;
            receivedParts.resize(packet_counts, false);
            data = malloc(size);

            this->socket = socket;
            this->endpoint = endpoint;
        }

        void processIncomingPacket(STIP_PACKET packet) override;

//    void sendAnswer(udp::socket &socket, udp::endpoint &endpoint, void *data, size_t size);

        void waitAprroval();

        int getStatus() const;

        std::string getDataAsString();

    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool isAnswered = false;
        void *answer = nullptr;
        size_t answerSize = 0;
        int status = -1;

        void *data = nullptr;
        size_t size = 0;
        size_t packet_counts = 0;

        std::vector<bool> receivedParts;

        udp::socket *socket = nullptr;
        udp::endpoint endpoint;
    };



// ------------------------------------------------ SessionManager.h ------------------------------------------------

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

}

#endif //RABBIT_SESSION_H
