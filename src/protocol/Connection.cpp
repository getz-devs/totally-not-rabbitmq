//
// Created by Serge on 08.03.2024.
//

#include "Connection.h"
#include "protocol/STIP.h"

Connection::Connection(udp::endpoint &endpoint, udp::socket *socket) {
    this->endpoint = endpoint;
    this->socket = socket;
    this->connectionStatus = 100;
    this->sessionManager = new SessionManager();
}

void Connection::addPacket(const STIP_PACKET &packet) {
    std::lock_guard<std::mutex> lock(mtx);
    packetQueue.push(packet);
//    timeoutTimer.reset();
    cv.notify_one();
    std::cout << "Packet added to queue" << std::endl;
}

STIP_PACKET Connection::getPacket(bool &result) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !packetQueue.empty(); });
    if (packetQueue.empty() ) {
        result = false;
        return {};
    }
    result = true;
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

    // delete session manager
    delete sessionManager;

    std::cout << "Connection destroyed" << std::endl;

}

void Connection::setConnectionStatus(char status) {
    connectionStatus = status;
}

void Connection::sendData(void *data, size_t size) {
    size_t packet_count = size / MAX_STIP_DATA_SIZE + 1;
    // TODO: Realize packet sending
}

uint32_t Connection::pingVersion() {
    uint32_t session_id = sessionManager->generateSessionId();
    auto *session = new PingSession(session_id);
    sessionManager->addSession(session);

    STIP_PACKET packet[1] = {};
    packet[0].header.command = 10;
    packet[0].header.session_id = session_id;
    packet[0].header.size = sizeof(STIP_HEADER);

    socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);


    uint32_t result = session->waitAnswer();
    sessionManager->deleteSession(session);

    delete session;
    return result;

    // TODO: Add timeout and  error handling
}

void Connection::processThread() {
    std::cout << "Start processing packets for " << endpoint.address() << ":" << endpoint.port() << std::endl;
    while (isRunning) {
        bool result = false;
        STIP_PACKET packet = getPacket(result);

        if (!result) continue;

        std::cout << "Command: " << packet.header.command << std::endl;
        std::cout << "Size: " << packet.header.size << std::endl;
        std::cout << "Session id: " << packet.header.session_id << std::endl;


        switch (packet.header.command) {
            case 10:
                // ping
                std::cout << "Ping received" << std::endl;
                PingSession::serverAnswer(*socket, endpoint, packet.header.session_id);
                break;
            default:
                auto tempSession = sessionManager->getSession(packet.header.session_id);
                if (tempSession != nullptr) {
                    tempSession->processIncomingPacket(packet);
                }
                break;
        }
    }
    std::cout << "Stop processing packets for " << endpoint.address() << ":" << endpoint.port() << std::endl;
}

void Connection::startProcessing() {
    if (isRunning) {
        return;
    }
    isRunning = true;
    mainThread = std::thread(&Connection::processThread, this);
}

void Connection::stopProcessing() {
    if (!isRunning) {
        return;
    }
    isRunning = false;
    cv.notify_one();
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
//    std::lock_guard<std::mutex> lock(mtx);
    for (auto &pair: connections) {
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
