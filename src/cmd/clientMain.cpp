//
// Created by Serge on 23.03.2024.
//

#include "rabbitCore/client/RabbitClient.h"
#include "DataModel/TaskRequest.h"
#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>

std::string promptSimpleMathTask() {
    json data;
    int num;

    std::cout << "Enter number one:" << std::endl;
    std::cout << "> ";
    std::cin >> num;
    if (std::cin.fail()) {
        std::cin.clear(); // Clear the error flag
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore invalid input
        throw std::runtime_error("Invalid input for number one.");
    }
    data["a"] = num;

    std::cout << "Enter number two:" << std::endl;
    std::cout << "> ";
    std::cin >> num;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::runtime_error("Invalid input for number two.");
    }
    data["b"] = num;

    return data.dump();
}


std::string promptMatrixDeterminantTask() {
    int matrixSize, matrixCount;
    std::cout << "Enter matrix size:" << std::endl;
    std::cout << "> ";
    std::cin >> matrixSize;
    std::cout << "Enter matrix count:" << std::endl;
    std::cout << "> ";
    std::cin >> matrixCount;

    std::vector<std::vector<std::vector<int>>> matrices;

    for (int i = 0; i < matrixCount; i++) {
        std::vector<std::vector<int>> matrix;
        for (int j = 0; j < matrixSize; j++) {
            std::vector<int> row;
            for (int k = 0; k < matrixSize; k++) {
                row.push_back(int(rand() % 100));
            }
            matrix.push_back(row);
        }
        matrices.push_back(matrix);
    }

    // размер кол-во элемент элемент
    // 10     100    23 23 23 23

    json data = json::array();
    for (auto &matrix: matrices) {
        json matrixJson = json::array();
        for (auto &row: matrix) {
            json rowJson = json::array();
            for (int &elem: row) {
                rowJson.push_back(elem);
            }
            matrixJson.push_back(rowJson);
        }
        data.push_back(matrixJson);
    }
    return data.dump();
}

void *receiverThread(void *arg) {
    auto *client = static_cast<RabbitClient *>(arg);
    client->receiveResutls();
    return nullptr;
}


void *senderThread(void *arg) {
    auto *client = static_cast<RabbitClient *>(arg);
    while (true) {
        std::cout << "Enter task number:" << std::endl;
        std::cout << "1 - simple math" << std::endl;
        std::cout << "2 - matrix multiplication" << std::endl;
        std::cout << "3 - bulk simple task" << std::endl;
        std::cout << "0 - exit console" << std::endl;
        std::cout << "> ";

        int taskNum;
        std::cin >> taskNum;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a valid task number." << std::endl;
            continue;
        }

        std::string requestFunc;
        std::string requestParams;
        int cores = 1;

        try {
            switch (taskNum) {
                case 1:
                    requestParams = promptSimpleMathTask();
                    requestFunc = "simpleMath";
                    break;

                case 2:
                    requestParams = promptMatrixDeterminantTask();
                    requestFunc = "determinant";
                    break;

                case 3:
                    int t, tasksCount, delay;
                    std::cout << "Enter tasks count:" << std::endl;
                    std::cout << "> ";
                    std::cin >> tasksCount;
                    std::cout << "Enter delay between tasks:" << std::endl;
                    std::cout << "> ";
                    std::cin >> delay;

                    for (t = 0; t < tasksCount; t++) {
                        json data;
                        data["a"] = rand() % 100;
                        data["b"] = rand() % 100;
                        int taskId = rand();
                        client->sendTask(TaskRequest{std::to_string(taskId), "simpleMath", data.dump(), cores});
                        std::cout << "Task " << taskId << " sent" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    }

                    requestParams = promptMatrixDeterminantTask();
                    requestFunc = "determinant";
                    break;

                case 0:
                    return nullptr;

                default:
                    std::cout << "Unknown task number" << std::endl;
                    continue;
            }

            TaskRequest tr{std::to_string(rand()), requestFunc, requestParams, cores};
            client->sendTask(tr);
        } catch (const std::runtime_error &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return nullptr;
}

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser program("RabbitClient");
    program.add_argument("-i", "--id")
            .help("Worker ID")
            .default_value("1234");
    program.add_argument("-h", "--host")
            .help("Server Host")
            .default_value("localhost");
    program.add_argument("-p", "--port")
            .help("Server Port")
            .default_value(12345)
            .action([](const std::string &value) { return std::stoi(value); });
    program.add_description("RabbitClient");
    program.add_epilog("RabbitClient is a client for Rabbit project");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        return 0;
    }

    auto id = program.get<std::string>("--id");
    auto host = program.get<std::string>("--host");
    int port = program.get<int>("--port");
    RabbitClient client(id, host, port);
    client.init();

    pthread_t receiverThreadId, senderThreadId;
    pthread_create(&receiverThreadId, nullptr, receiverThread, &client);
    pthread_create(&senderThreadId, nullptr, senderThread, &client);

    pthread_join(receiverThreadId, nullptr);
    pthread_join(senderThreadId, nullptr);

    return 0;
}
