//
// Created by Serge on 07.03.2024.
//


#include "STIPClient.h"
#include <future>
#include <chrono>

#include "protocol/errors/STIP_errors.h"

using namespace std::chrono_literals;

namespace STIP {

    STIPClient::STIPClient(udp::socket &socket) {
        this->socket = &socket;
        this->connectionManager = new ConnectionManager(socket);
    }


    void STIPClient::receiveProcess() {
        std::cout << "Start listen" << std::endl;
        while (isRunning) {
            STIP_PACKET packet[1];
            boost::system::error_code error;
            udp::endpoint remote_endpoint;
            size_t length = socket->receive_from(boost::asio::buffer(packet), remote_endpoint, 0, error);
            if (error && error != boost::asio::error::message_size) {
                return;
//            throw boost::system::system_error(error);
            }
            std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port()
                      << std::endl;

            connectionManager->accept(remote_endpoint, packet[0]);
        }
    }


    Connection *STIPClient::connect(udp::endpoint &targetEndpoint, std::chrono::milliseconds timeout) {

        auto *connection = new Connection(targetEndpoint, socket);
        this->connectionManager->addConnection(targetEndpoint, connection);


        STIP_PACKET initPacket[1] = {};
        initPacket[0].header.command = 100;
        initPacket[0].header.size = sizeof(int);

        this->socket->send_to(boost::asio::buffer(initPacket, initPacket[0].header.size), targetEndpoint);
        std::future<bool> response_future = std::async(std::launch::async, [connection]() {
            bool result = false;
            STIP_PACKET response{};
            do {
                response = connection->getPacket(result);
            } while (result && response.header.command != 101);

            return result;
        });

        std::future_status status;
        do {
            switch (status = response_future.wait_for(timeout); status) {
                case std::future_status::timeout:
                    connection->cancelPacketWaiting();
                    connectionManager->remove(targetEndpoint);
                    delete connection;
                    throw STIP::errors::STIPTimeoutException("Timeout while connecting");
                    break;
            }
        } while (status == std::future_status::deferred);

        if (response_future.get()) {
            initPacket[0].header.command = 102;
            initPacket[0].header.size = sizeof(int);
            this->socket->send_to(boost::asio::buffer(initPacket, initPacket[0].header.size), targetEndpoint);
            connection->setConnectionStatus(102);
            connection->startProcessing();
            return connection;
        }

    }

    Connection *STIPClient::connect(udp::endpoint &targetEndpoint) {
        return connect(
                targetEndpoint,
                std::chrono::milliseconds(10000)
        );
    }

    void STIPClient::startListen() {
        if (this->isRunning) {
            return;
        }
        this->isRunning = true;
        this->mainThread = std::thread(&STIPClient::receiveProcess, this);
    }

    void STIPClient::stopListen() {
        if (!this->isRunning) {
            return;
        }
        this->isRunning = false;
        socket->cancel();
        this->mainThread.join();
    }

}