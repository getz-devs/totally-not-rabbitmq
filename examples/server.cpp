//
// Created by Serge on 07.03.2024.
//

#include "protocol/STIP.h"
#include "server/STIPServer.h"

using boost::asio::ip::udp;


int main() {
    try {
        boost::asio::io_context io_context;

        // Создаем UDP сокет для приема запросов на порту 12345
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));


        STIPServer server(socket);

        for (;;) {
            Connection *connection = server.acceptConnection();
            std::cout << "Connection accepted\n\n" << std::endl;
        }


    } catch (std::exception &e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
