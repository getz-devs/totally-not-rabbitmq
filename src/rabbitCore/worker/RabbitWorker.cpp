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

using boost::asio::ip::udp;

RabbitWorker::RabbitWorker(std::string host, int port, int cores) {
    this->host = std::move(host);
    this->port = port;
    this->cores = cores;
}

void RabbitWorker::init() {
    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), host, std::to_string(port)).begin();

    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
    server_socket->open(udp::v4());

    STIP::STIPClient client(*server_socket);
    client.startListen();

    connection = client.connect(server_endpoint);
}

void RabbitWorker::startPolling() {
//    STIPServer server(*server_socket);
//    STIP::ReceiveMessageSession *received = connection->receiveMessage();

    for (;;) {
        STIP::ReceiveMessageSession *received = connection->receiveMessage();
        json request = received->getDataAsString();
        json data = request["data"];
        int taskCores = request["cores"];
        int func = request["func"];

        switch (func) {
            case 1: // simpleMath
                simpleMathHandler(data, taskCores);
                break;
            case 2: // determinant
                determinantHandler(data, taskCores);
                break;
            default:
                break;
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
    int det = 0;
    std::vector<std::vector<int>> submatrix(n);
    if (n == 2)
        return ((matrix[0][0] * matrix[1][1]) - (matrix[1][0] * matrix[0][1]));
    else {
        for (int x = 0; x < n; x++) {
            int subi = 0;
            for (int i = 1; i < n; i++) {
                int subj = 0;
                for (int j = 0; j < n; j++) {
                    if (j == x)
                        continue;
                    submatrix[subi][subj] = matrix[i][j];
                    subj++;
                }
                subi++;
            }
            det = det + (pow(-1, x) * matrix[0][x] * determinant( submatrix, n - 1 ));
        }
    }
    return det;
}

// handlers

void RabbitWorker::simpleMathHandler(json data, int taskCores) {

}

void RabbitWorker::determinantHandler(json data, int taskCores) {
    std::vector<std::thread> threads;
    threads.reserve(taskCores);

    std::vector<std::vector<std::vector<int>>> matrices = data.get<std::vector<std::vector<std::vector<int>>>>();

    std::vector<int> results(matrices.size());

    for (int i = 0; i < matrices.size(); ++i) {
        threads.emplace_back([this, &matrices, &results, i] {
                results[i] = this->determinant(matrices[i], matrices[i].size());
        });

        if (threads.size() == taskCores || i == matrices.size() - 1) {
            for (auto &t : threads) {
                t.join();
            }
            threads.clear();
        }
    }

    json jResults = json::array();
    for (int & result : results) {
        jResults.push_back(result);
    }
    TaskResult taskResult{0, jResults.dump(), 1};

    json response = taskResult;
    connection->sendMessage(response.dump());
}




