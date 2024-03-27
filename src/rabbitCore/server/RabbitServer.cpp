//
// Created by Serge on 23.03.2024.
//
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "RabbitServer.h"

using namespace STIP;

using boost::asio::ip::udp;



RabbitServer::RabbitServer(int port) {
    this->port = port;
}

void RabbitServer::init() {


    // Создаем UDP сокет для приема запросов на порту 12345
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
//    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));

    auto storage = make_storage("db.sqlite",
                                make_table("users",
                                           make_column("id",
                                                       &User::id,
                                                       autoincrement(),
                                                       primary_key()),
                                           make_column("first_name",
                                                       &User::firstName),
                                           make_column("last_name",
                                                       &User::lastName),
                                           make_column("birth_date",
                                                       &User::birthDate),
                                           make_column("image_url",
                                                       &User::imageUrl),
                                           make_column("type_id",
                                                       &User::typeId)),
                                make_table("user_types",
                                           make_column("id",
                                                       &UserType::id,
                                                       autoincrement(),
                                                       primary_key()),
                                           make_column("name",
                                                       &UserType::name,
                                                       default_value("name_placeholder"))));
}

void RabbitServer::startPolling() {
    STIPServer server(*server_socket);

    for (;;) {
        Connection *connection = server.acceptConnection();
        std::cout << "Connection accepted\n\n" << std::endl;
        std::thread(&RabbitServer::processConnection, this, connection).detach();
    }
}

void RabbitServer::processConnection(STIP::Connection *connection) {
    std::cout << "Connection accepted\n\n" << std::endl;

    std::string message = "Hello, I'm Ilya";
    connection->sendMessage(message);

//    ReceiveMessageSession *received = connection->receiveMessage();
//    std::cout << "Received message: " << received->getDataAsString() << std::endl;
}
