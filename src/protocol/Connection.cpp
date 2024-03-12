//
// Created by Serge on 08.03.2024.
//

#include "Connection.h"
#include "protocol/STIP.h"

EndpointConnection::EndpointConnection(udp::endpoint endpoint, const udp::socket &socket, ConnectionManager &connectionManager) {
    this->endpoint = endpoint;
    this->socket = &socket;
    this->connectionManager = &connectionManager;
    timeoutTimer.setTimeout(5000);
    timeoutTimer.setCallback([this] {
            if (packetQueue.empty()) {
                std::cout << "Connection timeout" << std::endl;
                timeoutTimer.stop();
                cv.notify_one();
                delete this;
            }
    });
    timeoutTimer.start();

    mainThread = std::thread([this] {
        while (true) {
            try {
                STIP_PACKET packet = getPacket();
                // print
                std::cout << "\n\nPacket received" << std::endl;
                std::cout << "Command: " << packet.header.command << std::endl;
                std::cout << "Session ID: " << packet.header.session_id << std::endl;
                std::cout << "\n\n";

                switch (packet.header.command) {
                    case 10:
                    case 0:
                        // check if session exists
                        // if not, create new session
                        if (sessions.find(packet.header.session_id) == sessions.end()) {
                            sessions[packet.header.session_id] = Session();
                        }
                        break;
                    default:
                        throw std::runtime_error("Unknown command");
                }
                std::cout << "Packet received" << std::endl;
            } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
                this->connectionManager->remove(this->endpoint);
                break;
            }

        }
    });

}

void EndpointConnection::addPacket(const STIP_PACKET &packet) {
    std::lock_guard<std::mutex> lock(mtx);
    packetQueue.push(packet);
//    timeoutTimer.reset();
    cv.notify_one();
    std::cout << "Packet added to queue" << std::endl;
}

STIP_PACKET EndpointConnection::getPacket() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !packetQueue.empty() || !timeoutTimer.isRunning(); });
    if (packetQueue.empty() && !timeoutTimer.isRunning()) {
        throw std::runtime_error("Connection timeout");
        // stop this thread
    }
    STIP_PACKET packet = packetQueue.front();
    packetQueue.pop();
    return packet;
}

// destructor
EndpointConnection::~EndpointConnection() {
    // remove connection from manager

    // stop main thread
    if (mainThread.joinable()) {
        mainThread.join();
    }
    std::cout << "Connection destroyed" << std::endl;

}

// ---------------------------- ConnectionManager ----------------------------

void ConnectionManager::accept(const udp::endpoint &endpoint, const STIP_PACKET &packet) {
    std::lock_guard<std::mutex> lock(mtx);
    if (connections.find(endpoint) == connections.end()) {
        connections[endpoint] = new EndpointConnection(endpoint, *socket, *this);
        std::cout << "New connection" << std::endl;
    }
    connections[endpoint]->addPacket(packet);
    std::cout << "Active connections: " << connections.size() << std::endl;
}

ConnectionManager::ConnectionManager(const udp::socket &socket) {
    this->socket = &socket;
}

ConnectionManager::~ConnectionManager() {
    for (auto& pair : connections) {
        delete pair.second;
    }
}

void ConnectionManager::remove(const udp::endpoint &endpoint) {
    std::lock_guard<std::mutex> lock(mtx);
//    delete connections[endpoint];
    connections.erase(endpoint);
}
