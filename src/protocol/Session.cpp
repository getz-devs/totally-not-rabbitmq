//
// Created by Serge on 16.03.2024.
//

#include "Session.h"

uint32_t Session::getId() const {
    return id;
}

// ------------------------------------------------ PingSession.h ------------------------------------------------

void PingSession::processIncomingPacket(STIP_PACKET packet) {
    if (packet.header.command != 11) {
        return;
    }
    std::cout << "PingSession processIncomingPacket" << std::endl;
    answer = *(uint32_t *) packet.data;
    isAnswered = true;
    cv.notify_one();
}

uint32_t PingSession::waitAnswer() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return isAnswered; });
    return answer;
}

PingSession::~PingSession() {
    std::cout << "PingSession destroyed" << std::endl;
}

PingSession::PingSession(uint32_t id) {
    this->id = id;
    std::cout << "PingSession created with id " << id << "\n";
}

void PingSession::serverAnswer(udp::socket &socket, udp::endpoint &endpoint, uint32_t sessionId) {
    STIP_PACKET packet[1] = {};
    packet[0].header.command = 11;
    packet[0].header.session_id = sessionId;
    packet[0].data[0] = STIP_PROTOCOL_VERSION;
    packet[0].header.size = sizeof(STIP_HEADER) + 1;
    socket.send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
}

// ------------------------------------------------ MessageSession.h ------------------------------------------------





// ------------------------------------------------ SessionManager.h ------------------------------------------------

uint32_t SessionManager::generateSessionId() {
    // mutex
    std::lock_guard<std::mutex> lock(mtx);

    std::random_device rd;
    while (true) {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1000000);
        uint32_t id = dis(gen);
        if (sessions.find(id) == sessions.end()) {
            return id;
        }
    }
}

void SessionManager::addSession(Session *session) {
    std::lock_guard<std::mutex> lock(mtx);

    if (sessions.find(session->getId()) != sessions.end()) {
        throw std::runtime_error("Session with this id already exists");
    }
    sessions[session->getId()] = session;
}

Session *SessionManager::getSession(uint32_t id) {
    std::lock_guard<std::mutex> lock(mtx);

    if (sessions.find(id) == sessions.end()) {
        return nullptr;
    }
    return sessions[id];
}

void SessionManager::deleteSession(Session *session) {
    std::lock_guard<std::mutex> lock(mtx);

//    if (sessions.find(session->getId()) == sessions.end()) {
//        throw std::runtime_error("Session with this id not found");
//    }
    sessions.erase(session->getId());
}
