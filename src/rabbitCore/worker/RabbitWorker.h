//
// Created by Serge on 23.03.2024.
//

#ifndef RABBIT_RabbitWorker_H
#define RABBIT_RabbitWorker_H

#include <boost/asio.hpp>
#include "protocol/Connection.h"
#include <nlohmann/json.hpp>
#include <adoint_backcompat.h>
#include <vector>
#include <iostream>

using boost::asio::ip::udp;
using json = nlohmann::json;

class RabbitWorker {
public:
    RabbitWorker(std::string host, int port, int cores);

    void init();

    // - нужно?
    //   void startPolling();
    //   void processConnection(STIP::Connection *connection);
    //

    void startPolling();

    ~RabbitWorker() {
        delete server_socket;
    }

private:
    std::string host;
    int port;
    int cores;
    STIP::Connection *connection;

    boost::asio::io_context io_context;
    udp::socket *server_socket{};

    // json parseMessage(std::string message);
    static bool validateRequest(json request);


    // functions (demo for now)

    int simpleMath(int a, int b);
    int determinant(std::vector<std::vector<int>> matrix, int n);
    static void doWait(int seconds);

    void simpleMathHandler(json data, int taskCores);
    void determinantHandler(json data, int taskCores);

};


#endif //RABBIT_RabbitWorker_H
