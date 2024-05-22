//
// Created by Serge on 23.03.2024.
//
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "DataModel/Message.h"
#include "RabbitServer.h"
#include <queue>
#include <iomanip> // For std::put_time

using namespace STIP;

using boost::asio::ip::udp;

RabbitServer::RabbitServer(int port) {
    this->port = port;
    taskService = TaskService();
}

void RabbitServer::init() {
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
}

void RabbitServer::startPolling() {
    STIPServer server(*server_socket);
    std::cout << "Server started on port " << port << std::endl;

    for (;;) {
        Connection *connection = server.acceptConnection();
        std::cout << "Connection accepted\n\n" << std::endl;
        std::thread(&RabbitServer::processConnection, this, connection).detach();
    }
}

void RabbitServer::processConnection(STIP::Connection *connection) {
    std::cout << "Connection accepted\n\n" << std::endl;

    auto receiveMessage = connection->receiveMessage();
    json request = json::parse(receiveMessage->getDataAsString());
    std::cout << "Received message: " << request.dump() << std::endl;
    Message message;
    json data;

    try {
        message = request.template get<Message>();
        data = json::parse(message.data);
    } catch (json::exception &e) {
        std::cerr << "Error parsing message: " << e.what() << std::endl;
        return;
    }

    switch (message.action) {
        case MessageType::RegisterClient: {
            std::cout << logTime() << "Received Client registration request\n";
            Client client;
            try {
                client = data.template get<Client>();
                client.connection = connection;
                std::cout << logTime() << "Client registered: " << client.id << std::endl;
            } catch (json::exception &e) {
                std::cerr << logTime() << "Error parsing client: " << e.what() << std::endl;
                break;
            }

            userDBService.addClient(client);

            try {
                processClient(client);
            } catch (std::exception &e) {
                std::cerr << logTime() <<  "Error processing client: " << e.what() << std::endl;
            }

            userDBService.removeClient(client);
            std::cout << logTime() << "Client disconnected: " << client.id << std::endl;
            break;
        }

        case MessageType::RegisterWorker: {
            std::cout << logTime() << "Received Worker registration request\n";
            Worker worker;
            try {
                worker = data.template get<Worker>();
                worker.connection = connection;
                std::cout << logTime() << "Worker registered: " << worker.id << " (Cores: " << worker.cores << ")\n";
            } catch (json::exception &e) {
                std::cerr << logTime() << "Error parsing worker: " << e.what() << std::endl;
                break;
            }

            userDBService.addWorker(worker);

            try {
                processWorker(worker);
            } catch (std::exception &e) {
                std::cerr << "Error processing worker: " << e.what() << std::endl;
            }

            userDBService.removeWorker(worker);
            std::cout << logTime() << "Worker disconnected: " << worker.id << std::endl;
            break;
        }

        default:
            std::cerr << logTime() << "Unknown action: " << message.action << std::endl;
            break;
    }

//    connection->kill();
    delete receiveMessage;
    delete connection;
}

void RabbitServer::processWorker(Worker &worker) {
    for (;;) {
        auto receiveMessage = worker.connection->receiveMessage();
        json request = json::parse(receiveMessage->getDataAsString());
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
                std::cout << logTime() << "Received TaskResult from worker: " << worker.id << std::endl;
                Task task;
                try {
                    task = data.template get<Task>();
                    std::cout << logTime() << "Task marked as Ready: " << task.id << std::endl;
                } catch (json::exception &e) {
                    std::cerr << logTime() << "Error parsing task: " << e.what() << std::endl;
                    break;
                }

                task.status = TaskStatus::Ready;
                taskService.updateTask(task);
                worker.usedCores -= task.cores;
                userDBService.updateWorker(worker);

                Client client = userDBService.findClientByID(task.client_hash_id);
                Message result_message;
                result_message.action = MessageType::TaskResult;
                result_message.data = data;

                json result_message_json = result_message;
                client.connection->sendMessage(result_message_json.dump());
                std::cout << logTime() << "Sent TaskResult to client: " << client.id << std::endl;

                checkTaskQueue(worker);
                break;
            }

            default:
                std::cerr << logTime() << "Unknown action: " << message.action << std::endl;
                break;
        }
    }
}

void RabbitServer::processClient(Client &client) {
    for (;;) {
        auto receiveMessage = client.connection->receiveMessage();
        json request = json::parse(receiveMessage->getDataAsString());
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
                std::cout << logTime() << "Received TaskRequest from client: " << client.id << std::endl;
                Task task;
                try {
                    task = data.template get<Task>();
                    std::cout << logTime() << "Task added: " << task.id << " (Cores: " << task.cores << ")" << std::endl;
                } catch (json::exception &e) {
                    std::cerr << logTime() <<  "Error parsing task: " << e.what() << std::endl;
                    break;
                }
                taskService.addTask(task);
                std::thread(&RabbitServer::processTask, this, std::ref(task)).detach();
                break;
            }

            default:
                std::cerr << logTime() << "Unknown action: " << message.action << std::endl;
                break;
        }
    }
}

void RabbitServer::checkTaskQueue(Worker &worker) {
    Task pendingTask;
    if (pendingTasks.tryDequeue(pendingTask, worker.cores - worker.usedCores)) {
        std::cout << logTime() << "Assigning pending task " << pendingTask.id << " to worker "
                  << worker.id << "\n";

        Message message;
        message.action = MessageType::TaskRequest;
        json task_json = pendingTask;
        message.data = task_json;

        json message_json = message;
        worker.connection->sendMessage(message_json.dump());

        std::cout << logTime() << "Task " << pendingTask.id << " sent to worker " << worker.id << std::endl;
    }
}


void RabbitServer::processTask(Task &task) {
    Worker worker = userDBService.findMostFreeWorker(task.cores);

    if (worker.id.empty()) {
        pendingTasks.enqueue(task);
        std::cout << logTime() << "Task " << task.id << " added to queue.\n";
        return;
    }

    task.worker_hash_id = worker.id;
    task.status = TaskStatus::SentToWorker;
    taskService.updateTask(task);

    Message message;
    message.action = MessageType::TaskRequest;
    json task_json = task;
    message.data = task_json;

    json message_json = message;
    worker.connection->sendMessage(message_json.dump());
    std::cout << logTime() << "Task " << task.id << " sent to worker " << worker.id << std::endl;
}

std::string RabbitServer::logTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X: ");
    return ss.str();
}