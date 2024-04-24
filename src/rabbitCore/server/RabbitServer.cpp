//
// Created by Serge on 23.03.2024.
//
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "DataModel/Message.h"
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
    json request = receiveMessage->getDataAsString();
    Message message;
    json data;
    try {
        message = request.template get<Message>();
        data = message.data;
    } catch (json::exception &e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
        return;
    }

    switch (message.action) {
        case MessageType::RegisterClient: {
            Client client;
            try {
                client = data.template get<Client>();
                client.connection = connection;
            } catch (json::exception &e) {
                std::cerr << "Error parsing client: " << e.what() << std::endl;
                break;
            }

            userDBService.addClient(client);

            try {
                processClient(client);
            } catch (std::exception &e) {
                std::cerr << "Error processing client: " << e.what() << std::endl;
            }

            userDBService.removeClient(client);
            break;
        }

        case MessageType::RegisterWorker: {
            Worker worker;
            try {
                worker = data.template get<Worker>();
                worker.connection = connection;
            } catch (json::exception &e) {
                std::cerr << "Error parsing worker: " << e.what() << std::endl;
                break;
            }

            userDBService.addWorker(worker);

            try {
                processWorker(worker);
            } catch (std::exception &e) {
                std::cerr << "Error processing worker: " << e.what() << std::endl;
            }

            userDBService.removeWorker(worker);
            break;
        }

        default:
            std::cerr << "Unknown action: " << message.action << std::endl;
            break;
    }

//    connection->kill();
    delete receiveMessage;
    delete connection;
}

void RabbitServer::processWorker(Worker &worker) {
    for (;;) {
        auto receiveMessage = worker.connection->receiveMessage();
        json request = receiveMessage->getDataAsString();
        Message message;
        json data;
        try {
            message = request.template get<Message>();
            data = message.data;
        } catch (json::exception &e) {
            std::cerr << "Error parsing message: " << e.what() << std::endl;
            continue;
        }

        switch (message.action) {
            case MessageType::TaskResult: {
                Task task;
                try {
                    task = data.template get<Task>();
                } catch (json::exception &e) {
                    std::cerr << "Error parsing task: " << e.what() << std::endl;
                    break;
                }

                // TODO:
                // update task status
                // update worker status
                // find client who wait for this task
                // send task result to client
                // update task status

                break;
            }

            default:
                std::cerr << "Unknown action: " << message.action << std::endl;
                break;
        }
    }
}

void RabbitServer::processClient(Client &client) {
    for (;;) {
        auto receiveMessage = client.connection->receiveMessage();
        json request = receiveMessage->getDataAsString();
        Message message;
        json data;
        try {
            message = request.template get<Message>();
            data = message.data;
        } catch (json::exception &e) {
            std::cerr << "Error parsing message: " << e.what() << std::endl;
            continue;
        }

        switch (message.action) {
            case MessageType::TaskRequest: {
                Task task;
                try {
                    task = data.template get<Task>();
                } catch (json::exception &e) {
                    std::cerr << "Error parsing task: " << e.what() << std::endl;
                    break;
                }

                // TODO:
                // add task to task DB
                // update task status
                // find free worker
                // send task to worker
                // update task status
                break;
            }

            default:
                std::cerr << "Unknown action: " << message.action << std::endl;
                break;
        }
    }
}

