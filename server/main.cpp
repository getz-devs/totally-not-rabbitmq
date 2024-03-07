//
// Created by Serge on 05.03.2024.
//

#include "../protocol/STIP.h"

using boost::asio::ip::udp;

std::string make_pong_message() {
    return "Pong";
}


int main() {
    try {
        boost::asio::io_context io_context;

        // Создаем UDP сокет для приема запросов на порту 9876
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 12345));

        for (;;) {
            STIP_PACKET sample[1];

//            sample[0].command = 0x01;
//            sample[0].offset = 0x02;
//            std::array<char, 300> recv_buf; // Буфер для принимаемых данных
//            recv_buf.fill(0);

            udp::endpoint remote_endpoint; // Информация об удаленном клиенте


            // Принимаем данные от клиента
            socket.receive_from(boost::asio::buffer(sample), remote_endpoint);
//            socket.receive()

            std::cout << "Получен запрос от " << remote_endpoint.address() << ":" << remote_endpoint.port() << std::endl;
//            std::cout.write(recv_buf.data(), recv_buf.size());
            std::cout << "Размер: " << sizeof(sample) << std::endl;
            std::cout << "Команда: " << sample[0].header.command << std::endl;
            std::cout << "Смещение: " << sample[0].header.offset << std::endl;
            std::cout << "ID пакета: " << sample[0].header.packet_id << std::endl;


            std::cout << std::endl;
            // Определяем, что будем отправлять обратно клиенту
            std::string response = make_pong_message();

            // Отправляем ответ обратно на адрес клиента
            socket.send_to(boost::asio::buffer(response), remote_endpoint);
        }
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}
