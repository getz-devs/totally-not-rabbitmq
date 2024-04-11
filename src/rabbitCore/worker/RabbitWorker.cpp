//
// Created by Serge on 23.03.2024.
//

#include "RabbitWorker.h"

#include <utility>
#include "protocol/STIP.h"
#include "server/STIPServer.h"
#include "protocol/Connection.h"

using namespace STIP;

using boost::asio::ip::udp;

RabbitWorker::RabbitWorker(std::string host, int port, int cores) {
    this->host = std::move(host);
    this->port = port;
    this->cores = cores;
}

void RabbitWorker::init() {
    server_socket = new udp::socket(io_context, udp::endpoint(udp::v4(), port));
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
