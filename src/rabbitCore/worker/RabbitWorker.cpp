//
// Created by Den on 23.03.2024.
//

#include "RabbitWorker.h"

#include <utility>
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"
#include "client/STIPClient.h"

using boost::asio::ip::udp;

RabbitWorker::RabbitWorker(std::string host, int port, int cores) {
    this->host = std::move(host);
    this->port = port;
    this->cores = cores;
}

void RabbitWorker::init() {
    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint server_endpoint = *resolver.resolve(udp::v4(), host, std::to_string(port)).begin();

    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
    server_socket->open(udp::v4());

    STIP::STIPClient client(*server_socket);
    client.startListen();

    connection = client.connect(server_endpoint);
}

void RabbitWorker::startPolling() {
//    STIPServer server(*server_socket);
    STIP::ReceiveMessageSession *received = connection->receiveMessage();

    for (;;) {

    }

}

// worker function implementations

void RabbitWorker::doWait(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

int RabbitWorker::doSimpleMath(int a, int b) {
    int c = a + b;
    std::this_thread::sleep_for(std::chrono::seconds(c));
    return c;
}


