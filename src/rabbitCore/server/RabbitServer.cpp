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
    taskService = TaskService();
}

void RabbitServer::init() {
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
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

                checkTaskQueue(worker);
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
                taskService.addTask(task);
                std::thread(&RabbitServer::processTask, this, std::ref(task)).detach();
                break;
            }

            default:
                std::cerr << "Unknown action: " << message.action << std::endl;
                break;
        }
    }
}

void RabbitServer::checkTaskQueue(Worker &worker) {
    Task pendingTask;
    if (pendingTasks.tryDequeue(pendingTask, worker.cores - worker.usedCores)) {
        // Assign the dequeued task to the worker.
        std::cout << "Assigning pending task " << pendingTask.id << " to worker "
                  << worker.id << "\n";

        // Send the task to the worker (similar to processTask)
        Message message;
        message.action = MessageType::TaskRequest;
        json task_json = pendingTask; // Use pendingTask here
        message.data = task_json;

        json message_json = message;
        worker.connection->sendMessage(message_json.dump());
    }
}


void RabbitServer::processTask(Task &task) {
    Worker worker = userDBService.findMostFreeWorker(task.cores);

    if (worker.id.empty()) {
        pendingTasks.enqueue(task);
        std::cout << "Task " << task.id << " added to queue.\n";
        return; // Exit the function - no need to wait
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
}