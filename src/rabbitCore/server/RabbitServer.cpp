//
// Created by Serge on 23.03.2024.
//
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "RabbitServer.h"

using namespace STIP;

using boost::asio::ip::udp;

RabbitServer::RabbitServer(int port) {
    this->port = port;
}

void RabbitServer::init() {
    // Создаем UDP сокет для приема запросов на порту 12345
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
    // server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
}

void RabbitServer::startPolling() {
    STIPServer server(*server_socket);

    for (;;) {
        Connection *connection = server.acceptConnection();
        std::cout << "Connection accepted\n\n" << std::endl;
        std::thread(&RabbitServer::processConnection, this, connection).detach();
    }
}

void RabbitServer::processConnection(STIP::Connection *connection) {
    std::cout << "Connection accepted\n\n" << std::endl;

    auto receiveMessage = connection->receiveMessage();
    json request;

    // validate request
    if (validateRequest(request)) {
        // process request
        std::string action = request["action"];
        if (action == "register") {
            // register
            std::string type = request["type"];
            if (type == "worker") {
                // register worker
                // create worker session

            } else if (type == "client") {
                // register client
                // create client session

            }
        }
    }

    // TODO: тут нужны умные указатели по идее
    delete receiveMessage;
    delete connection;
}

bool RabbitServer::validateRequest(json request) {
    // Action is required
    std::string action;
    try {
        action = request["action"];
    } catch (json::exception &e) {
        std::cerr << "Error parsing action: " << e.what() << std::endl;
        return false;
    }

    if (action == "register") {
        // Type is required for register action
        std::string type;
        try {
            type = request["type"];

        } catch (json::exception &e) {
            std::cerr << "Error parsing type: " << e.what() << std::endl;
            return false;
        }
    } else if (action == "send") {
        std::string queue;
        try {
            queue = request["queue"];
        } catch (json::exception &e) {
            std::cerr << "Error parsing queue: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
        return false;
    }
    return true;
}

//void RabbitServer::clientProccess(STIP::Connection *connection) {
//    std::cout << "Connection accepted\n\n" << std::endl;
//
//    auto receiveMessage = connection->receiveMessage();
//    json request;
//
//    // validate request
//    if (validateRequest(request)) {
//        // process request
//        std::string action = request["action"];
//        if (action == "register") {
//            // register
//            std::string type = request["type"];
//            if (type == "worker") {
//                // register worker
//                // create worker session
//
//            } else if (type == "client") {
//                // register client
//                // create client session
//
//            }
//        }
//    }
//    delete receiveMessage;
//    delete connection;
//}

/*
 * Process connection
 * Занимается обработкой первичного соединения. Позже вызывает отдельно методы для обработки клиентов и воркеров
 */

