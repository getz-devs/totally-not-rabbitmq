//
// Created by Serge on 16.03.2024.
//

#include "Session.h"
#include <boost/asio.hpp>

namespace STIP {

    uint32_t Session::getId() const {
        return id;
    }

// ------------------------------------------------ PingSession.h ------------------------------------------------

    void PingSession::processIncomingPacket(STIP_PACKET packet) {
        if (packet.header.command != Command::PING_ANSWER ) {
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
        packet[0].header.command = Command::PING_ANSWER;
        packet[0].header.session_id = sessionId;
        packet[0].data[0] = STIP_PROTOCOL_VERSION;
        packet[0].header.size = sizeof(STIP_HEADER) + 1;
        socket.send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
    }

// ------------------------------------------------ MessageSession.h ------------------------------------------------

    void SendMessageSession::processIncomingPacket(STIP_PACKET packet) {
        switch (packet.header.command) {
            case Command::MSG_INIT_RESPONSE_SUCCESS:
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

                status = SendMessageStatuses::INIT_RESPONSE_SUCCESS;
                cv.notify_all();
                break;

            case Command::MSG_INIT_RESPONSE_FAILURE : // BAD
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

                status = SendMessageStatuses::INIT_RESPONSE_FAILURE;
                cv.notify_all();
                break;
            case Command::MSG_RESPONSE_RESEND: // Resend ask
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;
                status = DATA_RESPONSE_RESEND;
                cv.notify_all();
                break;
            case Command::MSG_RESPONSE_ALL_RECEIVED : // Success
//            std::cout << "MessageSession processIncomingPacket" << std::endl;
//            std::cout << "Message: " << packet.data << std::endl;

                status = DATA_RESPONSE_SUCCESS;
                cv.notify_all();
                break;
            default:
                break;
        }

    }

    bool SendMessageSession::initSend() {
        STIP_PACKET packet[1] = {};
        packet[0].header.command = Command::MSG_INIT_REQUEST;
        packet[0].header.session_id = id;
        packet[0].header.packet_id = packet_counts; // means packets count in this case

        memcpy(packet[0].data, &size, sizeof(size_t));
        packet[0].header.size = sizeof(STIP_HEADER) + sizeof(size_t);

        status = SendMessageStatuses::INIT_REQUEST_SENT;
        socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);

        std::unique_lock<std::mutex> lock(mtx);
        status = SendMessageStatuses::INIT;
        cv.wait(lock, [this] { return status == SendMessageStatuses::INIT_RESPONSE_SUCCESS || status == SendMessageStatuses::INIT_RESPONSE_FAILURE || _cancaled; });
        return true;
    }

    bool SendMessageSession::sendData() {
        status = SendMessageStatuses::DATA_REQEUST_SENT;
        for (int i = 0; i < packet_counts; i++) {
            sendPart(i);
        }

        return true;
    }

    void SendMessageSession::sendPart(size_t packet_id) {
        STIP_PACKET packet[1] = {};
        packet[0].header.command = Command::MSG_SEND_DATA_PART;
        packet[0].header.session_id = id;
        packet[0].header.packet_id = packet_id;

        size_t partSize = (packet_id == packet_counts - 1) ? size % MAX_STIP_DATA_SIZE : MAX_STIP_DATA_SIZE;
        memcpy(packet[0].data, (void *) ((char *) data + packet_id * MAX_STIP_DATA_SIZE), partSize);
        packet[0].header.size = sizeof(STIP_HEADER) + partSize;

        socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
    }

    bool SendMessageSession::waitApproval() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return (status == DATA_RESPONSE_SUCCESS) || (status == DATA_RESPONSE_FAILURE) || _cancaled; });
        return status == DATA_RESPONSE_SUCCESS;
    }

    void SendMessageSession::cancel() {
        _cancaled = true;
        cv.notify_all();
    }


    std::string ReceiveMessageSession::getDataAsString() {
        std::string result = static_cast<const char *>(data);
        result.resize(size);
        return result;
    }

    void ReceiveMessageSession::processIncomingPacket(STIP_PACKET packet) {
        STIP_PACKET packet_response[1] = {};
        switch (packet.header.command) {
            case Command::MSG_INIT_REQUEST:
                if (packet.header.session_id != id) {
                    return;
                }
                size = *(size_t *) packet.data;
                packet_counts = (size + MAX_STIP_DATA_SIZE - 1) / MAX_STIP_DATA_SIZE;
                data = malloc(size);
                receivedParts.resize(packet_counts);
                status = 1;

                packet_response[0].header.command = Command::MSG_INIT_RESPONSE_SUCCESS;
                packet_response[0].header.session_id = id;
                packet_response[0].header.size = sizeof(STIP_HEADER);
                socket->send_to(boost::asio::buffer(packet_response, packet_response[0].header.size), endpoint);
                break;
            case Command::MSG_SEND_DATA_PART:
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
            case Command::MSG_REQUEST_ALL_RECEIVED:
                if (packet.header.session_id != id) {
                    return;
                }

                uint32_t unreceived = countUnreceivedParts();
                if (unreceived == 0) {
                    packet_response[0].header.command = Command::MSG_RESPONSE_ALL_RECEIVED;
                    packet_response[0].header.session_id = id;
                    packet_response[0].header.size = sizeof(STIP_HEADER);
                    socket->send_to(boost::asio::buffer(packet_response, packet_response[0].header.size), endpoint);
                } else {
                    packet_response[0].header.command = Command::MSG_RESPONSE_RESEND;
                    packet_response[0].header.session_id = id;
                    // data:

                    packet_response[0].header.packet_id = unreceived;
                    packet_response[0].header.size = sizeof(STIP_HEADER) + sizeof(uint32_t)*unreceived;

                    // fill data
                    uint32_t temp_data[unreceived];
                    uint32_t pos = 0;
                    for (int i = 0; i < packet_counts; i++) {
                        if (!receivedParts[i]) {
                            temp_data[pos] = i;
                            pos++;
                        }
                    }
                    memcpy(packet_response[0].data, temp_data, sizeof(uint32_t)*unreceived);
                    socket->send_to(boost::asio::buffer(packet_response, packet_response[0].header.size), endpoint);
                }
                break;
        }

        if (std::all_of(receivedParts.begin(), receivedParts.end(), [](bool i) { return i; })) {
            status = 5;

            STIP_PACKET packet[1] = {};
            packet[0].header.command = Command::MSG_RESPONSE_ALL_RECEIVED;
            packet[0].header.session_id = id;
            packet[0].header.size = sizeof(STIP_HEADER);
            socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);
        }
    }

    int ReceiveMessageSession::getStatus() const {
        return status;
    }

    uint32_t ReceiveMessageSession::countUnreceivedParts() const {
        return std::count(receivedParts.begin(), receivedParts.end(), true);
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

    void SessionManager::deleteSessionById(uint32_t id) {
        std::lock_guard<std::mutex> lock(mtx);

//        if (sessions.find(id) == sessions.end()) {
////            throw std::runtime_error("Session with this id not found");
//        }
        sessions.erase(id);
    }

}



