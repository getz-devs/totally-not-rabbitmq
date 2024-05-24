//
// Created by Serge on 23.03.2024.
//

#include "rabbitCore/worker/RabbitWorker.h"
#include <argparse/argparse.hpp>
#include <iostream>

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser program("RabbitWorker");
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
    program.add_argument("-c", "--cores")
            .help("Max cores")
            .default_value(4)
            .action([](const std::string &value) { return std::stoi(value); });
    program.add_description("RabbitWorker");
    program.add_epilog("RabbitWorker is a worker for Rabbit project");

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
    int cores = program.get<int>("--cores");
    RabbitWorker worker(id, host, port, cores);
    worker.init();
    std::cout << "Worker [" << id << "] started on [" << host << ":" << port << "]" << std::endl;
    worker.startPolling();
    return 0;
}