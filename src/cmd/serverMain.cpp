//
// Created by Serge on 23.03.2024.
//

#include "rabbitCore/server/RabbitServer.h"
#include <argparse/argparse.hpp>
#include <iostream>

int main(int argc, const char *argv[]) {
//    std::cout << "Hello, World!" << std::endl;
    argparse::ArgumentParser program("RabbitServer");
    program.add_argument("-p", "--port")
            .help("Port to listen")
            .default_value(12345)
            .action([](const std::string &value) { return std::stoi(value); });
    program.add_description("RabbitServer");
    program.add_epilog("RabbitServer is a server for Rabbit project");
//    program.parse_args(argc, argv);
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        return 0;
    }

    int port = program.get<int>("--port");
    RabbitServer server(12345);
    server.init();
    server.startPolling();
    return 0;
}