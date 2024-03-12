//
// Created by Serge on 07.03.2024.
//

#include "protocol/STIP.h"
#include "server/server.h"

using boost::asio::ip::udp;

std::string make_pong_message() {
    return "Pong";
}


int main() {
    try {
        boost::asio::io_context io_context;

        // Создаем UDP сокет для приема запросов на порту 9876
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));


        STIPServer server(socket);
//        server.send_message()
        server.accept();
//        for (;;) {
//            STIP_PACKET sample[1] = {};
//
//
//            udp::endpoint remote_endpoint; // Информация об удаленном клиенте
//
//            // Принимаем данные от клиента
//            socket.receive_from(boost::asio::buffer(sample), remote_endpoint);
//
//
//            std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;
//
//
//
//            std::cout << std::endl;
//            // Определяем, что будем отправлять обратно клиенту
//            std::string response = make_pong_message();
//
//            // Отправляем ответ обратно на адрес клиента
//            socket.send_to(boost::asio::buffer(response), remote_endpoint);
//        }
//

    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
