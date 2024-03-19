//
// Created by Serge on 07.03.2024.
//

#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"

using boost::asio::ip::udp;

// func to async process connection
void processConnection(Connection *connection) {
    ReceiveMessageSession * received= connection->receiveMessage();
    std::cout << "Received message: " << received->getDataAsString() << std::endl;
    connection->sendMessage("Hello, I'm server");
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Создаем UDP сокет для приема запросов на порту 12345
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));


        STIPServer server(socket);

        for (;;) {
            Connection *connection = server.acceptConnection();
            std::cout << "Connection accepted\n\n" << std::endl;

            std::thread(processConnection, connection).detach();
        }


    } catch (std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
