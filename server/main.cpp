//
// Created by Serge on 05.03.2024.
//
#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string/case_conv.hpp>

using boost::asio::ip::udp;

std::string make_daytime_string() {
    // Возвращает текущее время в формате дня и времени
    // (например, "Thu Mar 07 12:35:15 2024")
    // Это можно заменить на свою логику формирования сообщения.
    // В данном примере используется текущее системное время.
    time_t now = time(0);
    std::cout << "Got request" << std::endl;
    return ctime(&now);
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Создаем объект ip::udp::socket для приема запросов на порту 13 (Daytime Protocol).
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 8888));

        for (;;) {
            std::array<char, 1> recv_buf;
            udp::endpoint remote_endpoint;

            // Ожидаем, пока клиент свяжется с нами.
            socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            // Определяем, что мы будем отправлять обратно клиенту.
            std::string message = make_daytime_string();

            // Отправляем ответ на удаленный адрес.
            socket.send_to(boost::asio::buffer(message), remote_endpoint);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
