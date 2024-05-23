//
// Created by Serge on 23.03.2024.
//

#include "RabbitWorker.h"

#include <utility>
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "client/STIPClient.h"
#include "DataModel/TaskResult.h"
#include "DataModel/Worker.h"
#include "DataModel/Message.h"

using boost::asio::ip::udp;

//typedef void (RabbitWorker::*func_type)(void);
//typedef std::map<std::string, func_type> func_map_type;

RabbitWorker::RabbitWorker(std::string id, std::string host, int port, int cores) {
    this->id = std::move(id);
    this->host = std::move(host);
    this->port = port;
    this->cores = cores;
}

void RabbitWorker::init() {
    resolver = new udp::resolver(io_context);
    auto endpoints = resolver->resolve(udp::v4(), host, std::to_string(port));
    server_endpoint = new udp::endpoint(*endpoints.begin());

    server_socket = new udp::socket(io_context);
    server_socket->open(udp::v4());

    client = new STIP::STIPClient(*server_socket);
    client->startListen();

    connection = client->connect(*server_endpoint);

    mapping["simpleMath"] = &RabbitWorker::simpleMathHandler;
    mapping["determinant"] = &RabbitWorker::determinantHandler;

    // Register worker
    if (connection) {
        // Create Worker object to send
        Worker worker = {
                id,
                cores,
                0,
                nullptr
        };

        nlohmann::json workerJson;
        to_json(workerJson, worker);

        Message message = {
                MessageType::RegisterWorker,
                workerJson.dump()
        };

        nlohmann::json messageJson;
        to_json(messageJson, message);

        std::string msg = messageJson.dump();
        connection->sendMessage(msg);
    } else {
        std::cerr << "Error: Failed to connect to server." << std::endl;
    }
}


void RabbitWorker::startPolling() {
    for (;;) {
        STIP::ReceiveMessageSession *received = connection->receiveMessage();
        json request = json::parse(received->getDataAsString());
        json data = request["data"];
        int taskCores = request["cores"];
//        int func = request["func"];
        std::string func = request["func"];
        std::string id = request["id"];

        if (mapping.find(func) != mapping.end()) {
            (this->*mapping[func])(id, data, taskCores);
        } else {
            std::cout << "Function not found" << std::endl;
        }
    }
}

// worker function implementations

void RabbitWorker::doWait(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

int RabbitWorker::simpleMath(int a, int b) {
    int c = a + b;
    std::this_thread::sleep_for(std::chrono::seconds(c));
    return c;
}


int RabbitWorker::determinant(std::vector<std::vector<int>> matrix, int n) {
    int det = 1;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            while (matrix[j][i] != 0) {
                int ratio = matrix[i][i] / matrix[j][i];
                for (int k = i; k < n; k++) {
                    matrix[i][k] -= ratio * matrix[j][k];
                }
                std::swap(matrix[i], matrix[j]);
                det *= -1;
            }
        }
        det *= matrix[i][i];
    }

    return det;
}


// handlers

//void RabbitWorker::simpleMathHandler(json data, int taskCores) {
//    std::vector<std::thread> threads;
//    threads.reserve(taskCores);
//
//    std::vector<std::pair<int, int>> pairs = data.get<std::vector<std::pair<int, int>>>();
//
//    std::vector<int> results(pairs.size());
//
//    for (int i = 0; i < pairs.size(); ++i) {
//        threads.emplace_back([this, &pairs, &results, i] {
//            results[i] = this->simpleMath(pairs[i].first, pairs[i].second);
//        });
//
//        if (threads.size() == taskCores || i == pairs.size() - 1) {
//            for (auto &t : threads) {
//                t.join();
//            }
//            threads.clear();
//        }
//    }
//
//    json jResults = json::array();
//    for (int & result : results) {
//        jResults.push_back(result);
//    }
//    TaskResult taskResult{0, jResults.dump(), 1};
//
//    json response = taskResult;
//    connection->sendMessage(response.dump());
//}

void RabbitWorker::simpleMathHandler(std::string id, json data, int taskCores) {
    int a = data["a"];
    int b = data["b"];
    int result = simpleMath(a, b);

    json jResult = json::object();
    jResult["result"] = result;
    struct TaskResult taskResult{id, jResult.dump(), 1};

    json response = taskResult;
    connection->sendMessage(response.dump());
}

void RabbitWorker::determinantHandler(std::string id, json data, int taskCores) {
    std::vector<std::thread> threads;
    threads.reserve(taskCores);

    std::vector<std::vector<std::vector<int>>> matrices = data.get<std::vector<std::vector<std::vector<int>>>>();

    std::vector<int> results(matrices.size());

    for (int i = 0; i < matrices.size(); ++i) {
        threads.emplace_back([this, &matrices, &results, i] {
            results[i] = this->determinant(matrices[i], matrices[i].size());
        });

        if (threads.size() == taskCores || i == matrices.size() - 1) {
            for (auto &t: threads) {
                t.join();
            }
            threads.clear();
        }
    }

    json jResults = json::array();
    for (int &result: results) {
        jResults.push_back(result);
    }
    struct TaskResult taskResult{id, jResults.dump(), 1};

    json response = taskResult;
    connection->sendMessage(response.dump());
}




