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
            bool registered = false;
            // register
            std::string type = request["type"];
            if (type == "worker") {
                // TODO:
                // register worker
                // create worker session
                // добавляем воркера в бд
                // registered = userDb.addWorker(request["userID"])
                // try   {
                // processWorker(connection);
                // }
                // теперь можно просто удалить инфу о юзере
                // userdb.remove(userID)
                // connection->kill();
                // выход
            } else if (type == "client") {
                // TODO из пункта выше
            }
        }
    }

    delete receiveMessage;
    delete connection;
}

bool RabbitServer::validateRequest(json request) {
    // Action is required

    // TODO : полностью переписать

//    std::string action;
//    try {
//        action = request["action"];
//    } catch (json::exception &e) {
//        std::cerr << "Error parsing action: " << e.what() << std::endl;
//        return false;
//    }
//
//    if (action == "register") {
//        // Type is required for register action
//        std::string type;
//        try {
//            type = request["type"];
//
//        } catch (json::exception &e) {
//            std::cerr << "Error parsing type: " << e.what() << std::endl;
//            return false;
//        }
//    } else if (action == "send") {
//        std::string queue;
//        try {
//            queue = request["queue"];
//        } catch (json::exception &e) {
//            std::cerr << "Error parsing queue: " << e.what() << std::endl;
//            return false;
//        }
//    } else {
//        std::cerr << "Unknown action: " << action << std::endl;
//        return false;
//    }
    return true;
}

void RabbitServer::processWorker(STIP::Connection *connection) {
//    std::cout << "Connection accepted\n\n" << std::endl;

    for (;;) {
        auto receiveMessage = connection->receiveMessage();
        json request = receiveMessage->getDataAsString();

        // TODO:
        // получили сообщение об усешно вып задаче
        // обновляем статус задачи
        // обновляем статус воркера (ядра)
        // определяем кто ждет результат этой задачи
        // отправляем ему htpekmnfn
    }
    delete receiveMessage;
    delete connection;
}

void RabbitServer::processClient(STIP::Connection *connection) {
//    std::cout << "Connection accepted\n\n" << std::endl;

    for (;;) {
        auto receiveMessage = connection->receiveMessage();
        json request = receiveMessage->getDataAsString();

        // TODO:
        // получили реквест на задачу
        // кладем ее в бд
        // ищем воркера который сможет ее выполнить
        // отправляем ему задачу
        // обновляем ее статус
    }
    delete receiveMessage;
    delete connection;
}

/*
 * Process connection
 * Занимается обработкой первичного соединения. Позже вызывает отдельно методы для обработки клиентов и воркеров
 */

