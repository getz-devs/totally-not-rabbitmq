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

std::string promptMatrixMultiplicationTask() {
    int matrixSize;
    std::cout << "Enter matrix size:" << std::endl;
    std::cout << "> ";
    std::cin >> matrixSize;

    // Optimized random matrix generation using pre-allocated memory
    std::vector<int> randomNumbers(matrixSize * matrixSize * 2);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 99);
    std::generate(randomNumbers.begin(), randomNumbers.end(), [&]() { return distrib(gen); });

    json data = json::array();
    int index = 0;
    for (int i = 0; i < 2; ++i) {
        json matrixJson = json::array();
        for (int j = 0; j < matrixSize; ++j) {
            json rowJson = json::array();
            for (int k = 0; k < matrixSize; ++k) {
                rowJson.emplace_back(randomNumbers[index++]);
            }
            matrixJson.emplace_back(rowJson);
        }
        data.emplace_back(matrixJson);
    }

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

    json data = json::array();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::cout << "Start randoming " << std::endl;
    for (int i = 0; i < matrixCount; ++i) {
        json matrixJson = json::array();
        for (int j = 0; j < matrixSize; ++j) {
            json rowJson = json::array();
            for (int k = 0; k < matrixSize; ++k) {
                rowJson.emplace_back(rand() % 100);
            }
            matrixJson.emplace_back(rowJson);
        }
        data.emplace_back(matrixJson);
    }
    std::cout << "End randoming" << std::endl;
    // measure end time
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << std::endl;
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
        std::cout << "2 - matrix determinant" << std::endl;
        std::cout << "3 - matrix multiplication" << std::endl;
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
                    requestParams = promptMatrixMultiplicationTask();
                    requestFunc = "matrixMultiplication";
                    break;

//                    int t, tasksCount, delay;
//                    std::cout << "Enter tasks count:" << std::endl;
//                    std::cout << "> ";
//                    std::cin >> tasksCount;
//                    std::cout << "Enter delay between tasks:" << std::endl;
//                    std::cout << "> ";
//                    std::cin >> delay;
//                    for (t = 0; t < tasksCount; t++) {
//                        json data;
//                        data["a"] = rand() % 100;
//                        data["b"] = rand() % 100;
//                        int taskId = rand();
//                        client->sendTask(TaskRequest{std::to_string(taskId), "simpleMath", data.dump(), cores});
//                        std::cout << "Task " << taskId << " sent" << std::endl;
//                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
//                    }
//
//                    requestParams = promptMatrixDeterminantTask();
//                    requestFunc = "determinant";
//                    break;

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
