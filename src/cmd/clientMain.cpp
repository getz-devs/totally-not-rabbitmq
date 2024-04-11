//
// Created by Serge on 23.03.2024.
//

#include "rabbitCore/client/RabbitClient.h"
#include <argparse/argparse.hpp>
#include <iostream>

std::string promptMatrixMultTask() {
    int matrixRows, matrixCols;
    std::cout << "Enter matrix rows:" << std::endl;
    std::cout << "> ";
    std::cin >> matrixRows;
    std::cout << "Enter matrix cols:" << std::endl;
    std::cout << "> ";
    std::cin >> matrixCols;
    int matrix[matrixRows][matrixCols];

    for (int i = 0; i < matrixRows; i++) {
        // enter row as str with space as delimiter
        std::string rowStr;
        std::cout << "Enter row " << i << " of matrix:" << std::endl;
        std::cout << "> ";
        std::cin >> rowStr;
        // split rowStr by space
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        int j = 0;
        while ((pos = rowStr.find(delimiter)) != std::string::npos) {
            token = rowStr.substr(0, pos);
            matrix[i][j] = std::stoi(token);
            rowStr.erase(0, pos + delimiter.length());
            j++;
        }
        matrix[i][j] = std::stoi(rowStr);
    }
}

std::string promptLongTask() {
    std::cout << "Enter delay:" << std::endl;
    std::cout << "> ";
    int delay;
    std::cin >> delay;

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
        std::cout << "1 - matrix multiplication" << std::endl;
        std::cout << "2 - simulate something cool" << std::endl;
        std::cout << "0 - exit console" << std::endl;
        std::cout << "> ";

        int taskNum;
        std::cin >> taskNum;
        std::string requestFunc;
        std::string requestParams;

        switch (taskNum) {
            case 1:
                requestFunc = "matrixMult";
                requestParams = promptMatrixMultTask();
                break;

            case 2:
                requestFunc = "longTask";
                requestParams = promptLongTask();
                break;

            case 0:
                return nullptr;

            default:
                std::cout << "Unknown task number" << std::endl;
                break;
        }
    }
    return nullptr;
}

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser program("RabbitClient");
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

    auto host = program.get<std::string>("--host");
    int port = program.get<int>("--port");
    RabbitClient client(host, port);
    client.init();

    pthread_t receiverThreadId, senderThreadId;
    pthread_create(&receiverThreadId, nullptr, receiverThread, &client);
    pthread_create(&senderThreadId, nullptr, senderThread, &client);

    pthread_join(receiverThreadId, nullptr);
    pthread_join(senderThreadId, nullptr);

    return 0;
}
