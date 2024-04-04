//
// Created by Serge on 23.03.2024.
//

#include "rabbitCore/client/RabbitClient.h"
#include <argparse/argparse.hpp>
#include <iostream>

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

    std::string host = program.get<std::string>("--host");
    int port = program.get<int>("--port");
    RabbitClient client(host, port);
    client.init();
//    client.startPolling();
    return 0;
}