//
// Created by Serge on 23.03.2024.
//

#include "RabbitClient.h"

#include <utility>
#include "protocol/STIP.h"
#include "client/STIPClient.h"
#include "protocol/Connection.h"
#include "DataModel/Client.h"
#include "Message.h"

using namespace STIP;

using boost::asio::ip::udp;

RabbitClient::RabbitClient(std::string id, std::string host, int port) {
    this->id = std::move(id);
    this->host = std::move(host);
    this->port = port;
}

void RabbitClient::init() {
    resolver = new udp::resolver(io_context);
    auto endpoints = resolver->resolve(udp::v4(), host, std::to_string(port));
    server_endpoint = new udp::endpoint(*endpoints.begin());

    server_socket = new udp::socket(io_context);
    server_socket->open(udp::v4());

    client = new STIP::STIPClient(*server_socket);
    client->startListen();

    connection = client->connect(*server_endpoint);

    // Register client
    if (connection) {
        // Create Client object to send
        Client clientInfo = {
                id,
                connection
        };

        nlohmann::json clientJson;
        to_json(clientJson, clientInfo);

        Message message = {
                MessageType::RegisterClient,
                clientJson.dump()
        };

        nlohmann::json messageJson;
        to_json(messageJson, message);

        std::string msg = messageJson.dump();
        connection->sendMessage(msg);
    } else {
        std::cerr << "Error: Failed to connect to server." << std::endl;
    }

////////////

//
//    try {
//        Message test = {
//                MessageType::Invalid,
//                "test"
//        };
//        json testJson = test;
//        connection->sendMessage(testJson.dump());
//    } catch (std::exception e) {
//        std::cerr << "Error sending Test message 2: " << e.what() << std::endl;
//        return;
//    }
}

void RabbitClient::receiveResutls() {
    for (;;) {
        STIP::ReceiveMessageSession *received = connection->receiveMessage();
        json result = json::parse(received->getDataAsString());
        json data = result["data"];

        if (data.is_array()) {
            for (auto &row: data) {
                std::cout << row << std::endl;
            }
        } else {
            std::cout << data << std::endl;

        }
    }
}

void RabbitClient::sendTask(struct TaskRequest t) {
    json task = t;
    Message m = {
            MessageType::TaskRequest,
            task.dump()
    };
    json message = m;
    connection->sendMessage(message.dump());
}

void RabbitClient::testMessage(std::string msg) {
    try {
        Message test = {
                MessageType::Invalid,
                msg
        };
        json testJson = test;
        connection->sendMessage(testJson.dump());
    } catch (std::exception e) {
        std::cerr << "Error sending Test message: " << e.what() << std::endl;
        return;
    }
}
