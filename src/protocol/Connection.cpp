//
// Created by Serge on 08.03.2024.
//

#include "Connection.h"
#include "protocol/STIP.h"
#include "protocol/Session.h"
#include "protocol/errors/STIP_errors.h"
#include <future>


namespace STIP {

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
#ifdef STIP_PROTOCOL_DEBUG
        std::cout << "Packet added to queue" << std::endl;
#endif
    }


    STIP_PACKET Connection::getPacket(bool &result) {
        std::unique_lock<std::mutex> lock(mtx);
        countPacketWaiting++;
        cv.wait(lock, [this] { return !packetQueue.empty() || cancelPacketWaitingFlag; });
        countPacketWaiting--;
        if (packetQueue.empty()) {
            result = false;
            return {};
        }
        result = true;
        STIP_PACKET packet = packetQueue.front();
        packetQueue.pop();
        return packet;
    }

    void Connection::cancelPacketWaiting() {
        cancelPacketWaitingFlag = true;
        while (countPacketWaiting > 0) {
            cv.notify_one();
        }
        cancelPacketWaitingFlag = false;
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

#ifdef STIP_PROTOCOL_DEBUG
        std::cout << "Connection destroyed" << std::endl;
#endif
    }


    void Connection::setConnectionStatus(char status) {
        connectionStatus = status;
    }


    uint32_t Connection::pingVersion() {
        uint32_t session_id = sessionManager->generateSessionId();
        auto *session = new PingSession(session_id);
        sessionManager->addSession(session);

        STIP_PACKET packet[1] = {};
        packet[0].header.command = Command::PING_ASK;
        packet[0].header.session_id = session_id;
        packet[0].header.size = sizeof(STIP_HEADER);

        socket->send_to(boost::asio::buffer(packet, packet[0].header.size), endpoint);


        uint32_t result = session->waitAnswer();
        sessionManager->deleteSession(session);

        delete session;
        return result;

        // TODO: Add timeout and  error handling !!! (next time i'll finished this part)
    }


    bool Connection::sendMessage(void *data, size_t size) {
        uint32_t session_id = sessionManager->generateSessionId();
        auto *session = new SendMessageSession(session_id, data, size, socket, endpoint);
        sessionManager->addSession(session);


        // Send init
        bool totalResult = true;
        try {
            bool resultTimout = false;
            bool initSendResult = session->initSendWrappedTimout(resultTimout, 2000, 3);
            if (resultTimout) {
                throw STIP::errors::STIPTimeoutException("No response for init message");
            }
            if (!initSendResult) {
                throw STIP::errors::STIPException("Rejected init message");
            }


            // Send data
            session->sendData();
//            session->
            // wait for approval
//        timedOut = false;
//        std::future<bool> approval_future = std::async(std::launch::async, [session]() {
//            return session->waitApproval();
//        });
//
//        do {
//            switch (status = approval_future.wait_for(std::chrono::milliseconds(5000)); status) {
//                case std::future_status::timeout:
//                    session->cancel();
//                    timedOut = true;
//                    break;
//            }
//        } while (status == std::future_status::deferred);
//
//        if (timedOut) {
//            sessionManager->deleteSession(session);
//            delete session;
//
//            throw STIP::errors::STIPTimeoutException("No response for data message");
//        }
            bool approvalResult = session->waitApprovalWrappedTimout(resultTimout, 3000, 6);
            if (resultTimout) {
                throw STIP::errors::STIPTimeoutException("No response for data message");
            }

            if (!approvalResult) {
                throw STIP::errors::STIPException("Interrupted data message");
            }

        } catch (STIP::errors::STIPException &e) {
            std::cerr << e.what() << std::endl;
            totalResult = false;
        }



        sessionManager->deleteSession(session);
        delete session;
        return totalResult;
    }


    bool Connection::sendMessage(const std::string &message) {
        return sendMessage((void *) message.c_str(), message.size());
    }

    ReceiveMessageSession *Connection::receiveMessage() {
        std::unique_lock<std::mutex> lock(messageMtx);

        while (messageQueue.empty() && isRunning) {
            messageCv.wait(lock);
        }
        if (messageQueue.empty()) {
            return nullptr;
        }
        ReceiveMessageSession *message = messageQueue.front();
        messageQueue.pop();
        return message;
    }


    void Connection::processThread() {
#ifdef STIP_PROTOCOL_DEBUG
        std::cout << "Start processing packets for " << endpoint.address() << ":" << endpoint.port() << std::endl;
#endif
        while (isRunning) {
            bool result = false;
            STIP_PACKET packet = getPacket(result);

            if (!result) continue;
#ifdef STIP_PROTOCOL_DEBUG
            std::cout << "Command: " << packet.header.command << std::endl;
            std::cout << "Size: " << packet.header.size << std::endl;
            std::cout << "Session id: " << packet.header.session_id << std::endl;
#endif
            ReceiveMessageSession *tempReceiveSession;
            switch (packet.header.command) {
                ReceiveMessageSession *tempMsgSession;
                size_t packet_counts;
                uint32_t session_id;
                case Command::MSG_INIT_REQUEST :
                    // check if session exists
                    if (sessionManager->getSession(packet.header.session_id) != nullptr) {
                        std::cout << "Session already exist" << std::endl;
                        continue;
                    }

                    // message
#ifdef STIP_PROTOCOL_DEBUG
                    std::cout << "Message received" << std::endl;
#endif
//                    packet_counts = *(size_t *) packet.data;
                    packet_counts = packet.header.packet_id;

                    tempMsgSession = new ReceiveMessageSession(packet.header.session_id,
                                                               *(size_t *) packet.data,
                                                               packet_counts,
                                                               socket, endpoint);
                    sessionManager->addSession(tempMsgSession);
                    tempMsgSession->processIncomingPacket(packet);
                    session_id = packet.header.session_id;
//                    sessionKiller.registerSessionTimeout(
//                            packet.header.session_id,
//                            20000,
//                            [this, session_id] {
//                                auto *temp = dynamic_cast<ReceiveMessageSession *>(sessionManager->getSession(
//                                        session_id));
//                                std::cout << "Deleting session " << session_id << std::endl;
//                                // sleep 5 seconds
//                                std::this_thread::sleep_for(std::chrono::seconds(5));
//                                sessionManager->deleteSessionById(session_id);
//                                delete temp;
//                            }
//                    ); TODO: BUG Это вызывает странные ошибки во время работы. Временно без клинера CRITICAL
                    break;

                case Command::MSG_SEND_DATA_PART:
                    tempReceiveSession = dynamic_cast<ReceiveMessageSession *>(sessionManager->getSession(
                            packet.header.session_id));
                    if (tempReceiveSession == nullptr) break;

                    tempReceiveSession->processIncomingPacket(packet);
                    sessionKiller.resetSessionTimeout(packet.header.session_id);

                    if (tempReceiveSession->getStatus() == 5) {
                        if (tempReceiveSession->dispatched) break;
                        tempReceiveSession->dispatched = true;
                        std::lock_guard<std::mutex> lock(messageMtx);

                        messageQueue.push(tempReceiveSession);
#ifdef STIP_PROTOCOL_DEBUG
                        std::cout << "Message received, should be notified" << std::endl;
#endif
                        messageCv.notify_one();
                    }
                    break;

                case Command::MSG_KILLED:
                    tempReceiveSession = dynamic_cast<ReceiveMessageSession *>(sessionManager->getSession(
                            packet.header.session_id));
                    if (tempReceiveSession==nullptr) break;
                    sessionKiller.deleteSessionTimeout(packet.header.session_id);
                    sessionManager->deleteSession(tempReceiveSession);
                    delete tempReceiveSession;
                    break;

                case Command::PING_ASK:
                    // ping
#ifdef STIP_PROTOCOL_DEBUG
                    std::cout << "Ping received" << std::endl;
#endif
                    PingSession::serverAnswer(*socket, endpoint, packet.header.session_id);
                    break;
                default:
                    auto tempSession = sessionManager->getSession(packet.header.session_id);
                    if (tempSession != nullptr) {
                        tempSession->processIncomingPacket(packet);
                    } else {
                        std::cerr << "Session not found" << std::endl;
                    }

                    break;
            }
        }
#ifdef STIP_PROTOCOL_DEBUG
        std::cout << "Stop processing packets for " << endpoint.address() << ":" << endpoint.port() << std::endl;
#endif
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
        cv.notify_all();
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
}
