//
// Created by Serge on 16.03.2024.
//

#include "Session.h"
#include <boost/asio.hpp>

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

void SendMessageSession::processIncomingPacket(STIP_PACKET packet) {
    switch (packet.header.command) {
        case 1:
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

            status = 1;
            cv.notify_all();
            break;

        case 2: // BAD
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

            status = 2;
            cv.notify_all();
            break;
        case 4: // Resend ask
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

            // TODO: Resend
            break;
        case 5: // Success
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

            status = 5;
            cv.notify_all();
            break;
        default:
            break;
    }

}

bool SendMessageSession::initSend() {
    STIP_PACKET packet[1] = {};
    packet[0].header.command = 0;
    packet[0].header.session_id = id;
    packet[0].header.packet_id = packet_counts; // means packets count in this case

    memcpy(packet[0].data, &size, sizeof(size_t));
    packet[0].header.size = sizeof(STIP_HEADER) + sizeof(size_t);

    socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);

    std::unique_lock<std::mutex> lock(mtx);
    status = 0;
    cv.wait(lock, [this] { return status == 1; });
    return true;
}

bool SendMessageSession::sendData() {
    for (int i = 0; i < packet_counts; i++) {
        sendPart(i);
    }

    return true;
}

void SendMessageSession::sendPart(size_t packet_id) {
    STIP_PACKET packet[1] = {};
    packet[0].header.command = 3;
    packet[0].header.session_id = id;
    packet[0].header.packet_id = packet_id;

    size_t partSize = (packet_id == packet_counts - 1) ? size % MAX_STIP_DATA_SIZE : MAX_STIP_DATA_SIZE;
    memcpy(packet[0].data, (void *) ((char *) data + packet_id * MAX_STIP_DATA_SIZE), partSize);
    packet[0].header.size = sizeof(STIP_HEADER) + partSize;

    socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
}

bool SendMessageSession::waitApproval() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return (status == 5) || status == 6; });
    return status == 5;
}


std::string ReceiveMessageSession::getDataAsString() {
    std::string result = static_cast<const char *>(data);
    result.resize(size);
    return result;
}

void ReceiveMessageSession::processIncomingPacket(STIP_PACKET packet) {
    STIP_PACKET packet_response[1] = {};
    switch (packet.header.command) {
        case 0:
            if (packet.header.session_id != id) {
                return;
            }
            size = *(size_t *) packet.data;
            packet_counts = (size + MAX_STIP_DATA_SIZE - 1) / MAX_STIP_DATA_SIZE;
            data = malloc(size);
            receivedParts.resize(packet_counts);
            status = 1;

            packet_response[0].header.command = 1;
            packet_response[0].header.session_id = id;
            packet_response[0].header.size = sizeof(STIP_HEADER);
            socket->send_to(boost::asio::buffer(packet_response, packet_response[0].header.size), endpoint);
            break;
        case 3:
            if (packet.header.packet_id >= packet_counts) {
                return;
            }
            if (receivedParts[packet.header.packet_id]) {
                return;
            }
            memcpy((void *) ((char *) data + packet.header.packet_id * MAX_STIP_DATA_SIZE), packet.data,
                   packet.header.size - sizeof(STIP_HEADER));
            receivedParts[packet.header.packet_id] = true;
            break;
    }

    if (std::all_of(receivedParts.begin(), receivedParts.end(), [](bool i) { return i; })) {
        status = 5;

        STIP_PACKET packet[1] = {};
        packet[0].header.command = 5;
        packet[0].header.session_id = id;
        packet[0].header.size = sizeof(STIP_HEADER);
        socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
    }
}

int ReceiveMessageSession::getStatus() const {
    return status;
}



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

