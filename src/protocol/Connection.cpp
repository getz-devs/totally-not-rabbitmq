//
// Created by Serge on 08.03.2024.
//

#include "Connection.h"
#include "protocol/STIP.h"

Connection::Connection(udp::endpoint &endpoint, udp::socket *socket) {
    this->endpoint = endpoint;
    this->socket = socket;
//    this->connectionManager = &connectionManager;
//    timeoutTimer.setTimeout(5000);
//    timeoutTimer.setCallback([this] {
//            if (packetQueue.empty()) {
//                std::cout << "Connection timeout" << std::endl;
//                timeoutTimer.stop();
//                cv.notify_one();
//                delete this;
//            }
//    });
//    timeoutTimer.start();
//
//    mainThread = std::thread([this] {
//        while (true) {
//            try {
//                STIP_PACKET packet = getPacket();
//                // print
//                std::cout << "\n\nPacket received" << std::endl;
//                std::cout << "Command: " << packet.header.command << std::endl;
//                std::cout << "Session ID: " << packet.header.session_id << std::endl;
//                std::cout << "\n\n";
//
////                switch (packet.header.command) {
////                    case 10:
////                    case 0:
////                        // check if session exists
////                        // if not, create new session
//////                        if (sessions.find(packet.header.session_id) == sessions.end()) {
//////                            sessions[packet.header.session_id] = Session();
//////                        }
////                        break;
////                    default:
////                        throw std::runtime_error("Unknown command");
////                }
//                std::cout << "Packet received" << std::endl;
//            } catch (std::runtime_error &e) {
//                std::cerr << e.what() << std::endl;
////                this->connectionManager->remove(this->endpoint);
//                break;
//            }
//
//        }
//    });

}

void Connection::addPacket(const STIP_PACKET &packet) {
    std::lock_guard<std::mutex> lock(mtx);
    packetQueue.push(packet);
//    timeoutTimer.reset();
    cv.notify_one();
    std::cout << "Packet added to queue" << std::endl;
}

STIP_PACKET Connection::getPacket() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !packetQueue.empty();});
    if (packetQueue.empty() && !timeoutTimer.isRunning()) {
        throw std::runtime_error("Connection timeout");
        // stop this thread
    }
    STIP_PACKET packet = packetQueue.front();
    packetQueue.pop();
    return packet;
}

// destructor
Connection::~Connection() {
    // remove connection from manager

    // stop main thread
    if (mainThread.joinable()) {
        mainThread.join();
    }
    std::cout << "Connection destroyed" << std::endl;

}

void Connection::setConnectionStatus(char status) {
    connectionStatus = status;
}

void Connection::sendData(void *data, size_t size) {
    size_t packet_count = size / MAX_STIP_DATA_SIZE + 1;


}





// ---------------------------- ConnectionManager ----------------------------

void ConnectionManager::accept(const udp::endpoint &endpoint, const STIP_PACKET &packet) {
    std::lock_guard<std::mutex> lock(mtx);
    if (connections.find(endpoint) != connections.end()) {
        connections[endpoint]->addPacket(packet);
    }
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

bool ConnectionManager::check(const udp::endpoint &endpoint) {
    std::lock_guard<std::mutex> lock(mtx);
    if (connections.find(endpoint) != connections.end()) {
        return true;
    }
    return false;
}

void ConnectionManager::addConnection(const udp::endpoint &endpoint, Connection *connection) {
    std::lock_guard<std::mutex> lock(mtx);
    connections[endpoint] = connection;
}

Connection *ConnectionManager::getConnection(const udp::endpoint &endpoint) {
    std::lock_guard<std::mutex> lock(mtx);
    if (connections.find(endpoint) != connections.end()) {
        return connections[endpoint];
    }
    return nullptr;
}
